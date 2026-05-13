#include "GAS/Skill/MASkillManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/Skill/Definition/MASkillAssembler.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
#include "Net/UnrealNetwork.h"

UMASkillManagerComponent::UMASkillManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMASkillManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMASkillManagerComponent, ReplicatedSkillSlotStacks, COND_OwnerOnly);
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

		const FGameplayAbilitySpec AbilitySpec(UMASkillAbility::StaticClass(), 1, static_cast<int32>(InputID), GetAssembledDefinition(InputID));
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
}

void UMASkillManagerComponent::PrepareSkillSlotStacksForUI()
{
	const TArray<EMAAbilityInputID> InputIDs = GatherUniqueSkillSlotInputIDs();
	SkillStacks.Reserve(InputIDs.Num());
	for (const EMAAbilityInputID InputID : InputIDs)
	{
		FindOrAddStack(InputID);
	}
}

bool UMASkillManagerComponent::AddDefinition(EMAAbilityInputID InputID, UMASkillDefinition* Definition)
{
	if (!CanMutateSkillStacks()) return false;
	if (!Definition) return false;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	NormalizeDefinitionSlots(SkillStack.SourceDefinitions);

	const int32 EmptyIndex = SkillStack.SourceDefinitions.IndexOfByPredicate([](const UMASkillDefinition* Candidate)
	{
		return Candidate == nullptr;
	});
	if (EmptyIndex == INDEX_NONE) return false;

	SkillStack.SourceDefinitions[EmptyIndex] = Definition;
	return RebuildSkill(InputID);
}

bool UMASkillManagerComponent::ReplaceDefinitionAt(
	EMAAbilityInputID InputID,
	int32 DefinitionIndex,
	UMASkillDefinition* NewDefinition,
	UMASkillDefinition*& OutPreviousDefinition)
{
	OutPreviousDefinition = nullptr;

	if (!CanMutateSkillStacks()) return false;
	if (!IsConfiguredSkillSlotInputID(InputID)) return false;
	if (!IsValidDefinitionSlotIndex(DefinitionIndex)) return false;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	NormalizeDefinitionSlots(SkillStack.SourceDefinitions);

	UMASkillDefinition* PreviousDefinition = SkillStack.SourceDefinitions[DefinitionIndex];
	SkillStack.SourceDefinitions[DefinitionIndex] = NewDefinition;

	if (!RebuildSkill(InputID))
	{
		SkillStack.SourceDefinitions[DefinitionIndex] = PreviousDefinition;
		RebuildSkill(InputID);
		return false;
	}

	OutPreviousDefinition = PreviousDefinition;
	return true;
}

bool UMASkillManagerComponent::RequestSwapDefinitionSlotsBetween(
	EMAAbilityInputID InputIDA,
	int32 IndexA,
	EMAAbilityInputID InputIDB,
	int32 IndexB)
{
	if (!IsValidDefinitionSlotIndex(IndexA) || !IsValidDefinitionSlotIndex(IndexB))
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
		return SwapDefinitionSlotsBetween(InputIDA, IndexA, InputIDB, IndexB);

	ServerSwapDefinitionSlotsBetween(InputIDA, IndexA, InputIDB, IndexB);
	return true;
}

bool UMASkillManagerComponent::RequestMoveDefinitionSlot(
	const TArray<TObjectPtr<UMASkillDefinition>>* SourceSlots,
	int32 SourceIndex,
	UActorComponent* TargetOwner,
	const TArray<TObjectPtr<UMASkillDefinition>>* TargetSlots,
	int32 TargetIndex)
{
	EMAAbilityInputID SourceInputID = EMAAbilityInputID::None;
	if (!FindInputIDForDefinitionSlots(SourceSlots, SourceInputID)) return false;
	if (!SourceSlots->IsValidIndex(SourceIndex) || !(*SourceSlots)[SourceIndex]) return false;
	if (!TargetOwner || !TargetSlots || TargetIndex == INDEX_NONE) return false;

	if (UMASkillManagerComponent* TargetSkillManager = Cast<UMASkillManagerComponent>(TargetOwner))
	{
		if (TargetSkillManager != this) return false;

		EMAAbilityInputID TargetInputID = EMAAbilityInputID::None;
		if (!FindInputIDForDefinitionSlots(TargetSlots, TargetInputID)) return false;

		return RequestSwapDefinitionSlotsBetween(SourceInputID, SourceIndex, TargetInputID, TargetIndex);
	}

	if (UMASkillModuleInventoryComponent* TargetInventory = Cast<UMASkillModuleInventoryComponent>(TargetOwner))
	{
		const TArray<TObjectPtr<UMASkillDefinition>>* InventorySlots = TargetInventory->GetModuleSlotsForUI();
		if (TargetSlots != InventorySlots) return false;

		return TargetInventory->RequestMoveSkillSlotToInventorySlot(SourceInputID, SourceIndex, TargetIndex);
	}

	return false;
}

bool UMASkillManagerComponent::SwapDefinitionSlotsBetween(
	EMAAbilityInputID InputIDA,
	int32 IndexA,
	EMAAbilityInputID InputIDB,
	int32 IndexB)
{
	if (!CanMutateSkillStacks()) return false;
	if (!IsValidDefinitionSlotIndex(IndexA) || !IsValidDefinitionSlotIndex(IndexB)) return false;
	if (!IsConfiguredSkillSlotInputID(InputIDA) || !IsConfiguredSkillSlotInputID(InputIDB)) return false;
	if (InputIDA == InputIDB && IndexA == IndexB) return true;

	FMASkillDefinitionStack& SkillStackA = FindOrAddStack(InputIDA);
	FMASkillDefinitionStack& SkillStackB = FindOrAddStack(InputIDB);

	NormalizeDefinitionSlots(SkillStackA.SourceDefinitions);
	NormalizeDefinitionSlots(SkillStackB.SourceDefinitions);

	TObjectPtr<UMASkillDefinition> PreviousDefinitionA = SkillStackA.SourceDefinitions[IndexA];
	TObjectPtr<UMASkillDefinition> PreviousDefinitionB = SkillStackB.SourceDefinitions[IndexB];
	Swap(SkillStackA.SourceDefinitions[IndexA], SkillStackB.SourceDefinitions[IndexB]);

	if (InputIDA == InputIDB)
	{
		if (RebuildSkill(InputIDA))
		{
			return true;
		}

		SkillStackA.SourceDefinitions[IndexA] = PreviousDefinitionA;
		SkillStackB.SourceDefinitions[IndexB] = PreviousDefinitionB;
		RebuildSkill(InputIDA);
		return false;
	}

	const bool bRebuiltA = RebuildSkill(InputIDA);
	const bool bRebuiltB = RebuildSkill(InputIDB);
	if (bRebuiltA && bRebuiltB)
	{
		return true;
	}

	SkillStackA.SourceDefinitions[IndexA] = PreviousDefinitionA;
	SkillStackB.SourceDefinitions[IndexB] = PreviousDefinitionB;
	RebuildSkill(InputIDA);
	RebuildSkill(InputIDB);
	return false;
}

void UMASkillManagerComponent::ServerSwapDefinitionSlotsBetween_Implementation(
	EMAAbilityInputID InputIDA,
	int32 IndexA,
	EMAAbilityInputID InputIDB,
	int32 IndexB)
{
	SwapDefinitionSlotsBetween(InputIDA, IndexA, InputIDB, IndexB);
}

const TArray<TObjectPtr<UMASkillDefinition>>* UMASkillManagerComponent::GetDefinitionSlotsForUI(EMAAbilityInputID InputID)
{
	if (!IsConfiguredSkillSlotInputID(InputID)) return nullptr;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	NormalizeDefinitionSlots(SkillStack.SourceDefinitions);
	return &SkillStack.SourceDefinitions;
}

bool UMASkillManagerComponent::FindInputIDForDefinitionSlots(
	const TArray<TObjectPtr<UMASkillDefinition>>* DefinitionSlots,
	EMAAbilityInputID& OutInputID) const
{
	OutInputID = EMAAbilityInputID::None;
	if (!DefinitionSlots) return false;

	for (const FMASkillDefinitionStack& SkillStack : SkillStacks)
	{
		if (&SkillStack.SourceDefinitions != DefinitionSlots) continue;

		OutInputID = SkillStack.InputID;
		return OutInputID != EMAAbilityInputID::None;
	}

	return false;
}

UMASkillDefinition* UMASkillManagerComponent::GetAssembledDefinition(EMAAbilityInputID InputID) const
{
	const FMASkillDefinitionStack* SkillStack = FindStack(InputID);
	return SkillStack ? SkillStack->AssembledDefinition : nullptr;
}

bool UMASkillManagerComponent::RebuildSkill(EMAAbilityInputID InputID)
{
	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);

	SkillStack.AssembledDefinition = FMASkillAssembler::Assemble(this, SkillStack.SourceDefinitions);
	RefreshAbilityDefinition(SkillStack);
	UpdateReplicatedSkillSlotStack(SkillStack);
	OnSkillSlotChanged.Broadcast(InputID);
	return SkillStack.AssembledDefinition != nullptr || !HasAnyDefinition(SkillStack.SourceDefinitions);
}

void UMASkillManagerComponent::RegisterAbilityHandle(EMAAbilityInputID InputID, FGameplayAbilitySpecHandle AbilityHandle, TSubclassOf<UMASkillAbility> AbilityClass)
{
	if (AbilityClass != UMASkillAbility::StaticClass()) return;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	SkillStack.AbilityHandle = AbilityHandle;

	if (!SkillStack.AssembledDefinition && FindSkillSlotStack(InputID))
	{
		RebuildSkill(InputID);
		return;
	}

	if (!SkillStack.AssembledDefinition) return;

	RefreshAbilityDefinition(SkillStack);
}

void UMASkillManagerComponent::UnregisterAbilityHandle(EMAAbilityInputID InputID, FGameplayAbilitySpecHandle AbilityHandle)
{
	FMASkillDefinitionStack* SkillStack = FindStack(InputID);
	if (!SkillStack) return;
	if (SkillStack->AbilityHandle != AbilityHandle) return;
	SkillStack->AbilityHandle = FGameplayAbilitySpecHandle();
}

FMASkillDefinitionStack* UMASkillManagerComponent::FindStack(EMAAbilityInputID InputID)
{
	return SkillStacks.FindByPredicate([InputID](const FMASkillDefinitionStack& SkillStack)
	{
		return SkillStack.InputID == InputID;
	});
}

const FMASkillDefinitionStack* UMASkillManagerComponent::FindStack(EMAAbilityInputID InputID) const
{
	return SkillStacks.FindByPredicate([InputID](const FMASkillDefinitionStack& SkillStack)
	{
		return SkillStack.InputID == InputID;
	});
}

FMASkillDefinitionStack& UMASkillManagerComponent::FindOrAddStack(EMAAbilityInputID InputID)
{
	if (FMASkillDefinitionStack* ExistingStack = FindStack(InputID))
	{
		NormalizeDefinitionSlots(ExistingStack->SourceDefinitions);
		return *ExistingStack;
	}

	// UI socket widgets may keep direct pointers to SourceDefinitions arrays.
	// Add all runtime stacks before binding UI, then mutate only array values while widgets are alive.
	ensureMsgf(
		!OnSkillSlotChanged.IsBound(),
		TEXT("Do not add skill definition stacks while UI widgets may hold direct slot array pointers."));
	FMASkillDefinitionStack& NewStack = SkillStacks.AddDefaulted_GetRef();
	NewStack.InputID = InputID;
	if (const FMASkillSlotStack* SkillSlotStack = FindSkillSlotStack(InputID))
	{
		CopyDefinitionSlots(NewStack.SourceDefinitions, SkillSlotStack->Definitions);
	}
	else
	{
		NormalizeDefinitionSlots(NewStack.SourceDefinitions);
	}
	return NewStack;
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

bool UMASkillManagerComponent::IsValidDefinitionSlotIndex(int32 Index)
{
	return Index >= 0 && Index < SkillModuleSlotCount;
}

void UMASkillManagerComponent::NormalizeDefinitionSlots(TArray<TObjectPtr<UMASkillDefinition>>& Definitions)
{
	if (Definitions.Num() != 0 && Definitions.Num() != SkillModuleSlotCount)
	{
		ensureMsgf(
			false,
			TEXT("Do not resize skill definition slots while UI widgets may hold direct slot array pointers."));
	}
	Definitions.SetNum(SkillModuleSlotCount);
}

void UMASkillManagerComponent::CopyDefinitionSlots(
	TArray<TObjectPtr<UMASkillDefinition>>& Target,
	const TArray<TObjectPtr<UMASkillDefinition>>& Source)
{
	Target.Reset();
	Target.SetNum(SkillModuleSlotCount);

	const int32 CopyCount = FMath::Min(Source.Num(), SkillModuleSlotCount);
	for (int32 Index = 0; Index < CopyCount; ++Index)
	{
		Target[Index] = Source[Index];
	}
}

bool UMASkillManagerComponent::HasAnyDefinition(const TArray<TObjectPtr<UMASkillDefinition>>& Definitions)
{
	return Definitions.ContainsByPredicate([](const UMASkillDefinition* Definition)
	{
		return Definition != nullptr;
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

bool UMASkillManagerComponent::CanMutateSkillStacks() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

void UMASkillManagerComponent::OnRep_ReplicatedSkillSlotStacks()
{
	ApplyReplicatedSkillSlotStacks();
}

void UMASkillManagerComponent::ApplyReplicatedSkillSlotStacks()
{
	for (const FMASkillSlotStack& ReplicatedSkillSlotStack : ReplicatedSkillSlotStacks)
	{
		if (ReplicatedSkillSlotStack.InputID == EMAAbilityInputID::None) continue;

		FMASkillDefinitionStack& SkillStack = FindOrAddStack(ReplicatedSkillSlotStack.InputID);
		CopyDefinitionSlots(SkillStack.SourceDefinitions, ReplicatedSkillSlotStack.Definitions);
		RebuildSkill(ReplicatedSkillSlotStack.InputID);
	}
}

void UMASkillManagerComponent::UpdateReplicatedSkillSlotStack(const FMASkillDefinitionStack& SkillStack)
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;
	if (SkillStack.InputID == EMAAbilityInputID::None) return;

	FMASkillSlotStack* ReplicatedSkillSlotStack = ReplicatedSkillSlotStacks.FindByPredicate(
		[&SkillStack](const FMASkillSlotStack& Candidate)
		{
			return Candidate.InputID == SkillStack.InputID;
		});

	if (!ReplicatedSkillSlotStack)
	{
		ReplicatedSkillSlotStack = &ReplicatedSkillSlotStacks.AddDefaulted_GetRef();
		ReplicatedSkillSlotStack->InputID = SkillStack.InputID;
	}

	CopyDefinitionSlots(ReplicatedSkillSlotStack->Definitions, SkillStack.SourceDefinitions);
}

void UMASkillManagerComponent::RefreshAbilityDefinition(FMASkillDefinitionStack& SkillStack)
{
	UMASkillAbility* SkillAbility = ResolveSkillAbility(SkillStack);
	if (!SkillAbility) return;

	SkillAbility->UpdateCurrentSkillDefinition(SkillStack.AssembledDefinition);
}

UMASkillAbility* UMASkillManagerComponent::ResolveSkillAbility(const FMASkillDefinitionStack& SkillStack) const
{
	if (!SkillStack.AbilityHandle.IsValid()) return nullptr;

	const IAbilitySystemInterface* AbilitySystemOwner = Cast<IAbilitySystemInterface>(GetOwner());
	if (!AbilitySystemOwner) return nullptr;

	UAbilitySystemComponent* AbilitySystemComponent = AbilitySystemOwner->GetAbilitySystemComponent();
	if (!AbilitySystemComponent) return nullptr;

	FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromHandle(SkillStack.AbilityHandle);
	if (!AbilitySpec) return nullptr;

	return Cast<UMASkillAbility>(AbilitySpec->GetPrimaryInstance());
}
