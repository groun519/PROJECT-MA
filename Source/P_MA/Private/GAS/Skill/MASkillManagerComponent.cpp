#include "GAS/Skill/MASkillManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/Skill/Definition/MASkillAssembler.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "Setting/MAGameSettings.h"
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
	DOREPLIFETIME(UMASkillManagerComponent, ActivePreviewElementalTag);
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

	for (const FGameplayTag& SlotTag : GatherUniqueSkillSlotTags())
	{
		RebuildSkill(SlotTag);
	}
}

void UMASkillManagerComponent::PrepareSkillSlotRuntimeStatesForUI()
{
	const TArray<FGameplayTag> SlotTags = GatherUniqueSkillSlotTags();
	SkillSlotRuntimeStates.Reserve(SlotTags.Num());
	for (const FGameplayTag& SlotTag : SlotTags)
	{
		FindOrAddSlotRuntimeState(SlotTag);
	}
}

bool UMASkillManagerComponent::ReplaceDefinitionAt(
	FGameplayTag SlotTag,
	int32 ModuleIndex,
	UMASkillDefinition* NewDefinition)
{
	if (!CanMutateSkillSlots()) return false;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return false;
	if (!IsValidModuleSlotIndex(ModuleIndex)) return false;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	NormalizeModuleInstanceSlots(SlotState.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstance = SlotState.SourceModuleInstances[ModuleIndex];
	SlotState.SourceModuleInstances[ModuleIndex] = UMASkillModuleInstance::Create(GetOwner(), NewDefinition);

	if (!RebuildSkill(SlotTag))
	{
		SlotState.SourceModuleInstances[ModuleIndex] = PreviousModuleInstance;
		RebuildSkill(SlotTag);
		return false;
	}

	return true;
}

bool UMASkillManagerComponent::ReplaceModuleInstanceAt(
	FGameplayTag SlotTag,
	int32 ModuleIndex,
	UMASkillModuleInstance* NewModuleInstance,
	UMASkillModuleInstance*& OutPreviousModuleInstance)
{
	OutPreviousModuleInstance = nullptr;

	if (!CanMutateSkillSlots()) return false;
	if (!IsKnownSkillSlotTag(SlotTag)) return false;
	if (!IsValidModuleSlotIndex(ModuleIndex)) return false;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	NormalizeModuleInstanceSlots(SlotState.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstance = SlotState.SourceModuleInstances[ModuleIndex];
	SlotState.SourceModuleInstances[ModuleIndex] = NewModuleInstance;

	if (!RebuildSkill(SlotTag))
	{
		SlotState.SourceModuleInstances[ModuleIndex] = PreviousModuleInstance;
		RebuildSkill(SlotTag);
		return false;
	}

	OutPreviousModuleInstance = PreviousModuleInstance;
	return true;
}

bool UMASkillManagerComponent::RequestSwapModuleSlotsBetween(
	FGameplayTag SlotTagA,
	int32 IndexA,
	FGameplayTag SlotTagB,
	int32 IndexB)
{
	if (!IsValidModuleSlotIndex(IndexA) || !IsValidModuleSlotIndex(IndexB)) return false;
	if (!IsKnownSkillSlotTag(SlotTagA) || !IsKnownSkillSlotTag(SlotTagB)) return false;

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	if (OwnerActor->HasAuthority())
	{
		return SwapModuleSlotsBetween(SlotTagA, IndexA, SlotTagB, IndexB);
	}

	ServerSwapModuleSlotsBetween(SlotTagA, IndexA, SlotTagB, IndexB);
	return true;
}

bool UMASkillManagerComponent::RequestMoveModuleSlot(
	const TArray<TObjectPtr<UMASkillModuleInstance>>* SourceSlots,
	int32 SourceIndex,
	UActorComponent* TargetOwner,
	const TArray<TObjectPtr<UMASkillModuleInstance>>* TargetSlots,
	int32 TargetIndex)
{
	FGameplayTag SourceSlotTag;
	if (!FindSlotTagForModuleSlots(SourceSlots, SourceSlotTag)) return false;
	if (!SourceSlots->IsValidIndex(SourceIndex) || !(*SourceSlots)[SourceIndex] || !(*SourceSlots)[SourceIndex]->IsValid()) return false;
	if (!TargetOwner || !TargetSlots || TargetIndex == INDEX_NONE) return false;

	if (UMASkillManagerComponent* TargetSkillManager = Cast<UMASkillManagerComponent>(TargetOwner))
	{
		if (TargetSkillManager != this) return false;

		FGameplayTag TargetSlotTag;
		if (!FindSlotTagForModuleSlots(TargetSlots, TargetSlotTag)) return false;

		return RequestSwapModuleSlotsBetween(SourceSlotTag, SourceIndex, TargetSlotTag, TargetIndex);
	}

	if (UMASkillModuleInventoryComponent* TargetInventory = Cast<UMASkillModuleInventoryComponent>(TargetOwner))
	{
		const TArray<TObjectPtr<UMASkillModuleInstance>>* InventorySlots = TargetInventory->GetModuleSlotsForUI();
		if (TargetSlots != InventorySlots) return false;

		return TargetInventory->RequestMoveSkillSlotToInventorySlot(SourceSlotTag, SourceIndex, TargetIndex);
	}

	return false;
}

bool UMASkillManagerComponent::SwapModuleSlotsBetween(
	FGameplayTag SlotTagA,
	int32 IndexA,
	FGameplayTag SlotTagB,
	int32 IndexB)
{
	if (!CanMutateSkillSlots()) return false;
	if (!IsValidModuleSlotIndex(IndexA) || !IsValidModuleSlotIndex(IndexB)) return false;
	if (!IsKnownSkillSlotTag(SlotTagA) || !IsKnownSkillSlotTag(SlotTagB)) return false;
	if (SlotTagA == SlotTagB && IndexA == IndexB) return true;

	FMASkillSlotRuntimeState& SlotStateA = FindOrAddSlotRuntimeState(SlotTagA);
	FMASkillSlotRuntimeState& SlotStateB = FindOrAddSlotRuntimeState(SlotTagB);

	NormalizeModuleInstanceSlots(SlotStateA.SourceModuleInstances);
	NormalizeModuleInstanceSlots(SlotStateB.SourceModuleInstances);

	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstanceA = SlotStateA.SourceModuleInstances[IndexA];
	TObjectPtr<UMASkillModuleInstance> PreviousModuleInstanceB = SlotStateB.SourceModuleInstances[IndexB];
	Swap(SlotStateA.SourceModuleInstances[IndexA], SlotStateB.SourceModuleInstances[IndexB]);

	if (SlotTagA == SlotTagB)
	{
		if (RebuildSkill(SlotTagA)) return true;

		SlotStateA.SourceModuleInstances[IndexA] = PreviousModuleInstanceA;
		SlotStateB.SourceModuleInstances[IndexB] = PreviousModuleInstanceB;
		RebuildSkill(SlotTagA);
		return false;
	}

	const bool bRebuiltA = RebuildSkill(SlotTagA);
	const bool bRebuiltB = RebuildSkill(SlotTagB);
	if (bRebuiltA && bRebuiltB) return true;

	SlotStateA.SourceModuleInstances[IndexA] = PreviousModuleInstanceA;
	SlotStateB.SourceModuleInstances[IndexB] = PreviousModuleInstanceB;
	RebuildSkill(SlotTagA);
	RebuildSkill(SlotTagB);
	return false;
}

void UMASkillManagerComponent::ServerSwapModuleSlotsBetween_Implementation(
	FGameplayTag SlotTagA,
	int32 IndexA,
	FGameplayTag SlotTagB,
	int32 IndexB)
{
	SwapModuleSlotsBetween(SlotTagA, IndexA, SlotTagB, IndexB);
}

const TArray<TObjectPtr<UMASkillModuleInstance>>* UMASkillManagerComponent::GetModuleSlotsForUI(FGameplayTag SlotTag)
{
	if (!IsKnownSkillSlotTag(SlotTag)) return nullptr;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	NormalizeModuleInstanceSlots(SlotState.SourceModuleInstances);
	return &SlotState.SourceModuleInstances;
}

bool UMASkillManagerComponent::FindSlotTagForModuleSlots(
	const TArray<TObjectPtr<UMASkillModuleInstance>>* ModuleSlots,
	FGameplayTag& OutSlotTag) const
{
	OutSlotTag = FGameplayTag();
	if (!ModuleSlots) return false;

	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		if (&SlotState.SourceModuleInstances != ModuleSlots) continue;

		OutSlotTag = SlotState.SlotTag;
		return FMASkillSystemStatics::IsSkillSlotTag(OutSlotTag);
	}

	return false;
}

UMASkillDefinition* UMASkillManagerComponent::GetAssembledDefinition(FGameplayTag SlotTag) const
{
	const FMASkillSlotRuntimeState* SlotState = FindSlotRuntimeState(SlotTag);
	const UMASkillModuleInstance* AssembledModuleInstance = SlotState ? SlotState->AssembledModuleInstance : nullptr;
	return AssembledModuleInstance ? AssembledModuleInstance->GetDefinition() : nullptr;
}

const UMASkillGenericDataAsset* UMASkillManagerComponent::GetGenericSkillDataAsset() const
{
	if (GenericSkillDataAsset) return GenericSkillDataAsset;

	const UMAGameSettings* GameSettings = UMAGameSettings::Get();
	return GameSettings ? GameSettings->GetDefaultSkillGenericDataAsset() : nullptr;
}

bool UMASkillManagerComponent::RebuildSkill(FGameplayTag SlotTag)
{
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return false;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);

	SlotState.AssembledModuleInstance = FMASkillAssembler::Assemble(this, SlotState.SourceModuleInstances);
	EnsureAbilityForSlot(SlotState);
	RefreshAbilityDefinition(SlotState);
	UpdateReplicatedSkillSlotRuntimeState(SlotState);
	OnSkillSlotChanged.Broadcast(SlotTag);
	return SlotState.AssembledModuleInstance != nullptr || !HasAnyModuleInstance(SlotState.SourceModuleInstances);
}

void UMASkillManagerComponent::RegisterAbilityHandle(FGameplayTag SlotTag, FGameplayAbilitySpecHandle AbilityHandle, TSubclassOf<UMASkillAbility> AbilityClass)
{
	if (AbilityClass != UMASkillAbility::StaticClass()) return;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) return;

	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	SlotState.AbilityHandle = AbilityHandle;

	if (!SlotState.AssembledModuleInstance)
	{
		RebuildSkill(SlotTag);
		return;
	}

	RefreshAbilityDefinition(SlotState);
}

void UMASkillManagerComponent::UnregisterAbilityHandle(FGameplayTag SlotTag, FGameplayAbilitySpecHandle AbilityHandle)
{
	FMASkillSlotRuntimeState* SlotState = FindSlotRuntimeState(SlotTag);
	if (!SlotState) return;
	if (SlotState->AbilityHandle != AbilityHandle) return;
	SlotState->AbilityHandle = FGameplayAbilitySpecHandle();
}

bool UMASkillManagerComponent::TryActivateSkill(FGameplayTag SlotTag)
{
	if (!IsKnownSkillSlotTag(SlotTag)) return false;
	FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(SlotTag);
	if (!EnsureAbilityForSlot(SlotState)) return false;
	if (!SlotState.AbilityHandle.IsValid()) return false;

	const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner());
	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner ? AbilitySystemOwner->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent) return false;

	SetActivePreviewElementalTagFromSlot(SlotState);
	if (const FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(SlotState.AbilityHandle))
	{
		if (AbilitySpec->IsActive()) return true;
	}
	if (AbilitySystemComponent->TryActivateAbility(SlotState.AbilityHandle)) return true;

	ClearActivePreviewElementalTag();
	return false;
}

void UMASkillManagerComponent::SetActivePreviewElementalTagFromSlot(const FMASkillSlotRuntimeState& SlotState)
{
	const UMASkillDefinition* SkillDefinition = SlotState.AssembledModuleInstance
		? SlotState.AssembledModuleInstance->GetDefinition()
		: nullptr;
	ActivePreviewElementalTag = SkillDefinition ? SkillDefinition->GetElementalTag() : FGameplayTag();

	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->ForceNetUpdate();
	}
}

void UMASkillManagerComponent::ClearActivePreviewElementalTag()
{
	if (!ActivePreviewElementalTag.IsValid()) return;

	ActivePreviewElementalTag = FGameplayTag();
	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->ForceNetUpdate();
	}
}

FMASkillSlotRuntimeState* UMASkillManagerComponent::FindSlotRuntimeState(FGameplayTag SlotTag)
{
	return SkillSlotRuntimeStates.FindByPredicate([SlotTag](const FMASkillSlotRuntimeState& SlotState)
	{
		return SlotState.SlotTag == SlotTag;
	});
}

const FMASkillSlotRuntimeState* UMASkillManagerComponent::FindSlotRuntimeState(FGameplayTag SlotTag) const
{
	return SkillSlotRuntimeStates.FindByPredicate([SlotTag](const FMASkillSlotRuntimeState& SlotState)
	{
		return SlotState.SlotTag == SlotTag;
	});
}

FMASkillSlotRuntimeState& UMASkillManagerComponent::FindOrAddSlotRuntimeState(FGameplayTag SlotTag)
{
	if (FMASkillSlotRuntimeState* ExistingState = FindSlotRuntimeState(SlotTag))
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
	NewState.SlotTag = SlotTag;
	NormalizeModuleInstanceSlots(NewState.SourceModuleInstances);
	return NewState;
}

FMASkillSlotStack* UMASkillManagerComponent::FindSkillSlotStack(FGameplayTag SlotTag)
{
	return SkillSlotStacks.FindByPredicate([SlotTag](const FMASkillSlotStack& SkillSlotStack)
	{
		return SkillSlotStack.SlotTag == SlotTag;
	});
}

const FMASkillSlotStack* UMASkillManagerComponent::FindSkillSlotStack(FGameplayTag SlotTag) const
{
	return SkillSlotStacks.FindByPredicate([SlotTag](const FMASkillSlotStack& SkillSlotStack)
	{
		return SkillSlotStack.SlotTag == SlotTag;
	});
}

bool UMASkillManagerComponent::IsKnownSkillSlotTag(FGameplayTag SlotTag) const
{
	return FMASkillSystemStatics::IsSkillSlotTag(SlotTag)
		&& (FindSkillSlotStack(SlotTag) || FindSlotRuntimeState(SlotTag));
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

TArray<FGameplayTag> UMASkillManagerComponent::GatherUniqueSkillSlotTags() const
{
	TArray<FGameplayTag> UniqueSlotTags;
	TSet<FGameplayTag> SeenSlotTags;

	for (const FMASkillSlotStack& SkillSlotStack : SkillSlotStacks)
	{
		const FGameplayTag& SlotTag = SkillSlotStack.SlotTag;
		if (!FMASkillSystemStatics::IsSkillSlotTag(SlotTag)) continue;

		if (SeenSlotTags.Contains(SlotTag))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("UMASkillManagerComponent ignored duplicate SkillSlotStacks entry for SlotTag %s on %s."),
				*SlotTag.ToString(),
				*GetNameSafe(GetOwner()));
			continue;
		}

		SeenSlotTags.Add(SlotTag);
		UniqueSlotTags.Add(SlotTag);
	}

	for (const FMASkillSlotRuntimeState& SlotState : SkillSlotRuntimeStates)
	{
		if (!FMASkillSystemStatics::IsSkillSlotTag(SlotState.SlotTag) || SeenSlotTags.Contains(SlotState.SlotTag)) continue;

		SeenSlotTags.Add(SlotState.SlotTag);
		UniqueSlotTags.Add(SlotState.SlotTag);
	}

	return UniqueSlotTags;
}

bool UMASkillManagerComponent::CanMutateSkillSlots() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

bool UMASkillManagerComponent::EnsureAbilityForSlot(FMASkillSlotRuntimeState& SlotState)
{
	if (SlotState.AbilityHandle.IsValid()) return true;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotState.SlotTag)) return false;

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return true;

	const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(OwnerActor);
	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner ? AbilitySystemOwner->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent) return false;

	for (const FGameplayAbilitySpec& AbilitySpec : AbilitySystemComponent->GetActivatableAbilities())
	{
		if (!AbilitySpec.Ability || AbilitySpec.Ability->GetClass() != UMASkillAbility::StaticClass()) continue;
		if (!AbilitySpec.DynamicAbilityTags.HasTagExact(SlotState.SlotTag)) continue;

		SlotState.AbilityHandle = AbilitySpec.Handle;
		return true;
	}

	const int32 SlotInputID = FMASkillSystemStatics::ResolveSlotInputID(SlotState.SlotTag);
	if (SlotInputID == INDEX_NONE) return false;

	FGameplayAbilitySpec AbilitySpec(UMASkillAbility::StaticClass(), 1, SlotInputID, nullptr);
	AbilitySpec.DynamicAbilityTags.AddTag(SlotState.SlotTag);
	SlotState.AbilityHandle = AbilitySystemComponent->GiveAbility(AbilitySpec);
	return SlotState.AbilityHandle.IsValid();
}

void UMASkillManagerComponent::OnRep_ReplicatedSkillSlotRuntimeStates()
{
	ApplyReplicatedSkillSlotRuntimeStates();
}

void UMASkillManagerComponent::ApplyReplicatedSkillSlotRuntimeStates()
{
	for (const FMASkillReplicatedSlotRuntimeState& ReplicatedSlotState : ReplicatedSkillSlotRuntimeStates)
	{
		if (!FMASkillSystemStatics::IsSkillSlotTag(ReplicatedSlotState.SlotTag)) continue;

		FMASkillSlotRuntimeState& SlotState = FindOrAddSlotRuntimeState(ReplicatedSlotState.SlotTag);
		SlotState.SourceModuleInstances.Reset();
		SlotState.SourceModuleInstances.SetNum(SkillModuleSlotCount);

		const int32 CopyCount = FMath::Min(ReplicatedSlotState.ModuleInstances.Num(), SkillModuleSlotCount);
		for (int32 Index = 0; Index < CopyCount; ++Index)
		{
			SlotState.SourceModuleInstances[Index] = ReplicatedSlotState.ModuleInstances[Index];
		}

		RebuildSkill(ReplicatedSlotState.SlotTag);
	}
}

void UMASkillManagerComponent::UpdateReplicatedSkillSlotRuntimeState(const FMASkillSlotRuntimeState& SlotState)
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;
	if (!FMASkillSystemStatics::IsSkillSlotTag(SlotState.SlotTag)) return;

	FMASkillReplicatedSlotRuntimeState* ReplicatedSlotState = ReplicatedSkillSlotRuntimeStates.FindByPredicate(
		[&SlotState](const FMASkillReplicatedSlotRuntimeState& Candidate)
		{
			return Candidate.SlotTag == SlotState.SlotTag;
		});

	if (!ReplicatedSlotState)
	{
		ReplicatedSlotState = &ReplicatedSkillSlotRuntimeStates.AddDefaulted_GetRef();
		ReplicatedSlotState->SlotTag = SlotState.SlotTag;
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
