#include "GAS/Skill/MASkillManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/Skill/Definition/MASkillAssembler.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"

UMASkillManagerComponent::UMASkillManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMASkillManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMASkillManagerComponent, ReplicatedSkillSlotRuntimeStates, COND_OwnerOnly);
}

bool UMASkillManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWrote = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	if (!RepFlags || !RepFlags->bNetOwner) return bWrote;

	for (const FMASkillReplicatedSlotRuntimeState& ReplicatedSlotState : ReplicatedSkillSlotRuntimeStates)
	{
		for (UMASkillModuleInstance* ModuleInstance : ReplicatedSlotState.ModuleInstances)
		{
			if (!ModuleInstance) continue;
			bWrote |= Channel->ReplicateSubobject(ModuleInstance, *Bunch, *RepFlags);
		}
	}

	return bWrote;
}

void UMASkillManagerComponent::InitializeGrantedAbilities()
{
	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(OwnerActor);
	if (!AbilitySystemOwner) return;

	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent();
	if (!AbilitySystemComponent) return;

	for (const EMAAbilityInputID InputID : GatherUniqueSkillSlotInputIDs())
	{
		RebuildSkill(InputID);

		FGameplayAbilitySpecHandle ExistingHandle;
		for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
		{
			if (AbilitySpec.InputID != static_cast<int32>(InputID)) continue;
			if (!AbilitySpec.Ability || AbilitySpec.Ability->GetClass() != UMASkillAbility::StaticClass()) continue;

			ExistingHandle = AbilitySpec.Handle;
			break;
		}

		if (ExistingHandle.IsValid())
		{
			RegisterAbilityHandle(InputID, ExistingHandle, UMASkillAbility::StaticClass());
			continue;
		}

		const FGameplayAbilitySpec AbilitySpec(UMASkillAbility::StaticClass(), 1, static_cast<int32>(InputID), nullptr);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

void UMASkillManagerComponent::PrepareSkillSlotRuntimeStatesForUI()
{
	const TArray<EMAAbilityInputID> InputIDs = GatherUniqueSkillSlotInputIDs();
	SkillSlotRuntimeStates.Reserve(InputIDs.Num());
	for (const EMAAbilityInputID InputID : InputIDs)
	{
		FindOrAddSlotRuntimeState(InputID);
	}
}

bool UMASkillManagerComponent::ReplaceDefinitionAt(
	EMAAbilityInputID InputID,
	int32 ModuleIndex,
	UMASkillDefinition* NewDefinition,
	UMASkillDefinition*& OutPreviousDefinition)
{
	OutPreviousDefinition = nullptr;

	if (!CanMutateSkillSlots()) return false;
	if (!IsConfiguredSkillSlotInputID(InputID)) return false;
	if (!IsValidModuleSlotIndex(ModuleIndex)) return false;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(InputID);
	NormalizeModuleInstanceSlots(SlotState.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstance = SlotState.SourceModuleInstances[ModuleIndex];
	SlotState.SourceModuleInstances[ModuleIndex] = UMASkillModuleInstance::Create(GetOwner(), NewDefinition);

	if (!RebuildSkill(InputID))
	{
		SlotState.SourceModuleInstances[ModuleIndex] = PreviousModuleInstance;
		RebuildSkill(InputID);
		return false;
	}

	OutPreviousDefinition = PreviousModuleInstance ? PreviousModuleInstance->GetDefinition() : nullptr;
	return true;
}

bool UMASkillManagerComponent::ReplaceModuleInstanceAt(
	EMAAbilityInputID InputID,
	int32 ModuleIndex,
	UMASkillModuleInstance* NewModuleInstance,
	UMASkillModuleInstance*& OutPreviousModuleInstance)
{
	OutPreviousModuleInstance = nullptr;

	if (!CanMutateSkillSlots()) return false;
	if (!IsConfiguredSkillSlotInputID(InputID)) return false;
	if (!IsValidModuleSlotIndex(ModuleIndex)) return false;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(InputID);
	NormalizeModuleInstanceSlots(SlotState.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstance = SlotState.SourceModuleInstances[ModuleIndex];
	SlotState.SourceModuleInstances[ModuleIndex] = NewModuleInstance;

	if (!RebuildSkill(InputID))
	{
		SlotState.SourceModuleInstances[ModuleIndex] = PreviousModuleInstance;
		RebuildSkill(InputID);
		return false;
	}

	OutPreviousModuleInstance = PreviousModuleInstance;
	return true;
}

bool UMASkillManagerComponent::RequestSwapModuleSlotsBetween(
	EMAAbilityInputID InputIDA,
	int32 IndexA,
	EMAAbilityInputID InputIDB,
	int32 IndexB)
{
	if (!IsValidModuleSlotIndex(IndexA) || !IsValidModuleSlotIndex(IndexB))
	{
		return false;
	}
	if (!IsConfiguredSkillSlotInputID(InputIDA) || !IsConfiguredSkillSlotInputID(InputIDB))
	{
		return false;
	}

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	if (OwnerActor->HasAuthority())
		return SwapModuleSlotsBetween(InputIDA, IndexA, InputIDB, IndexB);

	ServerSwapModuleSlotsBetween(InputIDA, IndexA, InputIDB, IndexB);
	return true;
}

bool UMASkillManagerComponent::RequestMoveModuleSlot(
	const TArray<TObjectPtr<UMASkillModuleInstance>>* SourceSlots,
	int32 SourceIndex,
	UActorComponent* TargetOwner,
	const TArray<TObjectPtr<UMASkillModuleInstance>>* TargetSlots,
	int32 TargetIndex)
{
	EMAAbilityInputID SourceInputID = EMAAbilityInputID::None;
	if (!FindInputIDForModuleSlots(SourceSlots, SourceInputID)) return false;
	if (!SourceSlots->IsValidIndex(SourceIndex) || !(*SourceSlots)[SourceIndex] || !(*SourceSlots)[SourceIndex]->IsValid()) return false;
	if (!TargetOwner || !TargetSlots || TargetIndex == INDEX_NONE) return false;

	if (UMASkillManagerComponent* TargetSkillManager = Cast<UMASkillManagerComponent>(TargetOwner))
	{
		if (TargetSkillManager != this) return false;

		EMAAbilityInputID TargetInputID = EMAAbilityInputID::None;
		if (!FindInputIDForModuleSlots(TargetSlots, TargetInputID)) return false;

		return RequestSwapModuleSlotsBetween(SourceInputID, SourceIndex, TargetInputID, TargetIndex);
	}

	if (UMASkillModuleInventoryComponent* TargetInventory = Cast<UMASkillModuleInventoryComponent>(TargetOwner))
	{
		const TArray<TObjectPtr<UMASkillModuleInstance>>* InventorySlots = TargetInventory->GetModuleSlotsForUI();
		if (TargetSlots != InventorySlots) return false;

		return TargetInventory->RequestMoveSkillSlotToInventorySlot(SourceInputID, SourceIndex, TargetIndex);
	}

	return false;
}

bool UMASkillManagerComponent::SwapModuleSlotsBetween(
	EMAAbilityInputID InputIDA,
	int32 IndexA,
	EMAAbilityInputID InputIDB,
	int32 IndexB)
{
	if (!CanMutateSkillSlots()) return false;
	if (!IsValidModuleSlotIndex(IndexA) || !IsValidModuleSlotIndex(IndexB)) return false;
	if (!IsConfiguredSkillSlotInputID(InputIDA) || !IsConfiguredSkillSlotInputID(InputIDB)) return false;
	if (InputIDA == InputIDB && IndexA == IndexB) return true;

	FMASkillSlotRuntimeState& SlotStateA = FindOrAddSlotRuntimeState(InputIDA);
	FMASkillSlotRuntimeState& SlotStateB = FindOrAddSlotRuntimeState(InputIDB);

	NormalizeModuleInstanceSlots(SlotStateA.SourceModuleInstances);
	NormalizeModuleInstanceSlots(SlotStateB.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstanceA = SlotStateA.SourceModuleInstances[IndexA];
	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstanceB = SlotStateB.SourceModuleInstances[IndexB];
	Swap(SlotStateA.SourceModuleInstances[IndexA], SlotStateB.SourceModuleInstances[IndexB]);

	if (InputIDA == InputIDB)
	{
		if (RebuildSkill(InputIDA))
		{
			return true;
		}

		SlotStateA.SourceModuleInstances[IndexA] = PreviousModuleInstanceA;
		SlotStateB.SourceModuleInstances[IndexB] = PreviousModuleInstanceB;
		RebuildSkill(InputIDA);
		return false;
	}

	const bool bRebuiltA = RebuildSkill(InputIDA);
	const bool bRebuiltB = RebuildSkill(InputIDB);
	if (bRebuiltA && bRebuiltB)
	{
		return true;
	}

	SlotStateA.SourceModuleInstances[IndexA] = PreviousModuleInstanceA;
	SlotStateB.SourceModuleInstances[IndexB] = PreviousModuleInstanceB;
	RebuildSkill(InputIDA);
	RebuildSkill(InputIDB);
	return false;
}

void UMASkillManagerComponent::ServerSwapModuleSlotsBetween_Implementation(
	EMAAbilityInputID InputIDA,
	int32 IndexA,
	EMAAbilityInputID InputIDB,
	int32 IndexB)
{
	SwapModuleSlotsBetween(InputIDA, IndexA, InputIDB, IndexB);
}

const TArray<TObjectPtr<UMASkillModuleInstance>>* UMASkillManagerComponent::GetModuleSlotsForUI(EMAAbilityInputID InputID)
{
	if (!IsConfiguredSkillSlotInputID(InputID)) return nullptr;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(InputID);
	NormalizeModuleInstanceSlots(SlotState.SourceModuleInstances);
	return &SlotState.SourceModuleInstances;
}

bool UMASkillManagerComponent::FindInputIDForModuleSlots(
	const TArray<TObjectPtr<UMASkillModuleInstance>>* ModuleSlots,
	EMAAbilityInputID& OutInputID) const
{
	OutInputID = EMAAbilityInputID::None;
	if (!ModuleSlots) return false;

	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		if (&SlotState.SourceModuleInstances != ModuleSlots) continue;

		OutInputID = SlotState.InputID;
		return OutInputID != EMAAbilityInputID::None;
	}

	return false;
}

UMASkillDefinition* UMASkillManagerComponent::GetAssembledDefinition(EMAAbilityInputID InputID) const
{
	const FMASkillSlotRuntimeState* SlotState = FindSlotRuntimeState(InputID);
	const UMASkillModuleInstance* AssembledModuleInstance = SlotState ? SlotState->AssembledModuleInstance : nullptr;
	return AssembledModuleInstance ? AssembledModuleInstance->GetDefinition() : nullptr;
}

bool UMASkillManagerComponent::RebuildSkill(EMAAbilityInputID InputID)
{
	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(InputID);

	SlotState.AssembledModuleInstance = FMASkillAssembler::Assemble(this, SlotState.SourceModuleInstances);
	RefreshAbilityDefinition(SlotState);
	UpdateReplicatedSkillSlotRuntimeState(SlotState);
	OnSkillSlotChanged.Broadcast(InputID);
	return SlotState.AssembledModuleInstance != nullptr || !HasAnyModuleInstance(SlotState.SourceModuleInstances);
}

void UMASkillManagerComponent::RegisterAbilityHandle(EMAAbilityInputID InputID, FGameplayAbilitySpecHandle AbilityHandle, TSubclassOf<UMASkillAbility> AbilityClass)
{
	if (AbilityClass != UMASkillAbility::StaticClass()) return;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(InputID);
	SlotState.AbilityHandle = AbilityHandle;

	if (!SlotState.AssembledModuleInstance && FindSkillSlotStack(InputID))
	{
		RebuildSkill(InputID);
		return;
	}

	if (!SlotState.AssembledModuleInstance) return;

	RefreshAbilityDefinition(SlotState);
}

void UMASkillManagerComponent::UnregisterAbilityHandle(EMAAbilityInputID InputID, FGameplayAbilitySpecHandle AbilityHandle)
{
	FMASkillSlotRuntimeState* SlotState = FindSlotRuntimeState(InputID);
	if (!SlotState) return;
	if (SlotState->AbilityHandle != AbilityHandle) return;
	SlotState->AbilityHandle = FGameplayAbilitySpecHandle();
}

FMASkillSlotRuntimeState* UMASkillManagerComponent::FindSlotRuntimeState(EMAAbilityInputID InputID)
{
	return SkillSlotRuntimeStates.FindByPredicate([InputID](const FMASkillSlotRuntimeState& SlotState)
	{
		return SlotState.InputID == InputID;
	});
}

const FMASkillSlotRuntimeState* UMASkillManagerComponent::FindSlotRuntimeState(EMAAbilityInputID InputID) const
{
	return SkillSlotRuntimeStates.FindByPredicate([InputID](const FMASkillSlotRuntimeState& SlotState)
	{
		return SlotState.InputID == InputID;
	});
}

FMASkillSlotRuntimeState& UMASkillManagerComponent::FindOrAddSlotRuntimeState(EMAAbilityInputID InputID)
{
	if (FMASkillSlotRuntimeState* ExistingState = FindSlotRuntimeState(InputID))
	{
		NormalizeModuleInstanceSlots(ExistingState->SourceModuleInstances);
		return *ExistingState;
	}

	// UI socket widgets may keep direct pointers to SourceModuleInstances arrays.
	// Add all runtime states before binding UI, then mutate only array values while widgets are alive.
	ensureMsgf(
		!OnSkillSlotChanged.IsBound(),
		TEXT("Do not add skill slot runtime states while UI widgets may hold direct slot array pointers."));
	FMASkillSlotRuntimeState& NewState = SkillSlotRuntimeStates.AddDefaulted_GetRef();
	NewState.InputID = InputID;
	NormalizeModuleInstanceSlots(NewState.SourceModuleInstances);
	return NewState;
}

FMASkillSlotStack* UMASkillManagerComponent::FindSkillSlotStack(EMAAbilityInputID InputID)
{
	return SkillSlotStacks.FindByPredicate([InputID](const FMASkillSlotStack& SkillSlotStack)
	{
		return SkillSlotStack.InputID == InputID;
	});
}

const FMASkillSlotStack* UMASkillManagerComponent::FindSkillSlotStack(EMAAbilityInputID InputID) const
{
	return SkillSlotStacks.FindByPredicate([InputID](const FMASkillSlotStack& SkillSlotStack)
	{
		return SkillSlotStack.InputID == InputID;
	});
}

bool UMASkillManagerComponent::IsConfiguredSkillSlotInputID(EMAAbilityInputID InputID) const
{
	return InputID != EMAAbilityInputID::None && FindSkillSlotStack(InputID) != nullptr;
}

bool UMASkillManagerComponent::IsValidModuleSlotIndex(int32 Index)
{
	return Index >= 0 && Index < SkillModuleSlotCount;
}

void UMASkillManagerComponent::NormalizeModuleInstanceSlots(TArray<TObjectPtr<UMASkillModuleInstance>>& ModuleInstances)
{
	if (ModuleInstances.Num() != 0 && ModuleInstances.Num() != SkillModuleSlotCount)
	{
		ensureMsgf(
			false,
			TEXT("Do not resize skill module slots while UI widgets may hold direct slot array pointers."));
	}
	ModuleInstances.SetNum(SkillModuleSlotCount);
}

bool UMASkillManagerComponent::HasAnyModuleInstance(const TArray<TObjectPtr<UMASkillModuleInstance>>& ModuleInstances)
{
	return ModuleInstances.ContainsByPredicate([](const UMASkillModuleInstance* ModuleInstance)
	{
		return ModuleInstance && ModuleInstance->IsValid();
	});
}

TArray<EMAAbilityInputID> UMASkillManagerComponent::GatherUniqueSkillSlotInputIDs() const
{
	TArray<EMAAbilityInputID> UniqueInputIDs;
	TSet<EMAAbilityInputID> SeenInputIDs;

	for (const FMASkillSlotStack& SkillSlotStack : SkillSlotStacks)
	{
		const EMAAbilityInputID InputID = SkillSlotStack.InputID;
		if (InputID == EMAAbilityInputID::None) continue;

		if (SeenInputIDs.Contains(InputID))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("UMASkillManagerComponent ignored duplicate SkillSlotStacks entry for InputID %d on %s."),
				static_cast<int32>(InputID),
				*GetNameSafe(GetOwner()));
			continue;
		}

		SeenInputIDs.Add(InputID);
		UniqueInputIDs.Add(InputID);
	}

	return UniqueInputIDs;
}

bool UMASkillManagerComponent::CanMutateSkillSlots() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

void UMASkillManagerComponent::OnRep_ReplicatedSkillSlotRuntimeStates()
{
	ApplyReplicatedSkillSlotRuntimeStates();
}

void UMASkillManagerComponent::ApplyReplicatedSkillSlotRuntimeStates()
{
	for (const FMASkillReplicatedSlotRuntimeState& ReplicatedSlotState : ReplicatedSkillSlotRuntimeStates)
	{
		if (ReplicatedSlotState.InputID == EMAAbilityInputID::None) continue;

		FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(ReplicatedSlotState.InputID);
		SlotState.SourceModuleInstances.Reset();
		SlotState.SourceModuleInstances.SetNum(SkillModuleSlotCount);

		const int32 CopyCount = FMath::Min(ReplicatedSlotState.ModuleInstances.Num(), SkillModuleSlotCount);
		for (int32 Index = 0; Index < CopyCount; ++Index)
		{
			SlotState.SourceModuleInstances[Index] = ReplicatedSlotState.ModuleInstances[Index];
		}

		RebuildSkill(ReplicatedSlotState.InputID);
	}
}

void UMASkillManagerComponent::UpdateReplicatedSkillSlotRuntimeState(const FMASkillSlotRuntimeState& SlotState)
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;
	if (SlotState.InputID == EMAAbilityInputID::None) return;

	FMASkillReplicatedSlotRuntimeState* ReplicatedSlotState = ReplicatedSkillSlotRuntimeStates.FindByPredicate(
		[&SlotState](const FMASkillReplicatedSlotRuntimeState& Candidate)
		{
			return Candidate.InputID == SlotState.InputID;
		});

	if (!ReplicatedSlotState)
	{
		ReplicatedSlotState = &ReplicatedSkillSlotRuntimeStates.AddDefaulted_GetRef();
		ReplicatedSlotState->InputID = SlotState.InputID;
	}

	ReplicatedSlotState->ModuleInstances.Reset();
	ReplicatedSlotState->ModuleInstances.SetNum(SkillModuleSlotCount);

	const int32 CopyCount = FMath::Min(SlotState.SourceModuleInstances.Num(), SkillModuleSlotCount);
	for (int32 Index = 0; Index < CopyCount; ++Index)
	{
		ReplicatedSlotState->ModuleInstances[Index] = SlotState.SourceModuleInstances[Index];
	}
}

void UMASkillManagerComponent::RefreshAbilityDefinition(FMASkillSlotRuntimeState& SlotState)
{
	UMASkillAbility* SkillAbility = ResolveSkillAbility(SlotState);
	if (!SkillAbility) return;

	SkillAbility->UpdateCurrentSkillModuleInstance(SlotState.AssembledModuleInstance);
}

UMASkillAbility* UMASkillManagerComponent::ResolveSkillAbility(const FMASkillSlotRuntimeState& SlotState) const
{
	if (!SlotState.AbilityHandle.IsValid()) return nullptr;

	const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner());
	if (!AbilitySystemOwner) return nullptr;

	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent();
	if (!AbilitySystemComponent) return nullptr;

	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(SlotState.AbilityHandle);
	if (!AbilitySpec) return nullptr;

	return Cast<UMASkillAbility>(AbilitySpec->GetPrimaryInstance());
}
