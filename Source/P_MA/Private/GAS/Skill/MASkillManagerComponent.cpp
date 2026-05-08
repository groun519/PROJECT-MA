#include "GAS/Skill/MASkillManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GAS/Skill/Definition/MASkillAssembler.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillAbility.h"
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

void UMASkillManagerComponent::SetDefinitions(EMAAbilityInputID InputID, const TArray<UMASkillDefinition*>& Definitions)
{
	if (!CanMutateSkillStacks()) return;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	CopyDefinitionSlots(SkillStack.SourceDefinitions, Definitions);
	RebuildSkill(InputID);
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

bool UMASkillManagerComponent::RemoveDefinitionAt(EMAAbilityInputID InputID, int32 RemoveIndex)
{
	if (!CanMutateSkillStacks()) return false;

	FMASkillDefinitionStack* SkillStack = FindStack(InputID);
	if (!SkillStack) return false;

	NormalizeDefinitionSlots(SkillStack->SourceDefinitions);
	if (!IsValidDefinitionSlotIndex(RemoveIndex)) return false;
	if (!SkillStack->SourceDefinitions[RemoveIndex]) return true;

	SkillStack->SourceDefinitions[RemoveIndex] = nullptr;
	return RebuildSkill(InputID);
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

	FindOrAddStack(InputIDA);
	FindOrAddStack(InputIDB);

	FMASkillDefinitionStack* SkillStackA = FindStack(InputIDA);
	FMASkillDefinitionStack* SkillStackB = FindStack(InputIDB);
	if (!SkillStackA || !SkillStackB) return false;

	NormalizeDefinitionSlots(SkillStackA->SourceDefinitions);
	NormalizeDefinitionSlots(SkillStackB->SourceDefinitions);

	Swap(SkillStackA->SourceDefinitions[IndexA], SkillStackB->SourceDefinitions[IndexB]);

	if (InputIDA == InputIDB)
	{
		return RebuildSkill(InputIDA);
	}

	const bool bRebuiltA = RebuildSkill(InputIDA);
	const bool bRebuiltB = RebuildSkill(InputIDB);
	return bRebuiltA || bRebuiltB;
}

void UMASkillManagerComponent::ServerSwapDefinitionSlotsBetween_Implementation(
	EMAAbilityInputID InputIDA,
	int32 IndexA,
	EMAAbilityInputID InputIDB,
	int32 IndexB)
{
	SwapDefinitionSlotsBetween(InputIDA, IndexA, InputIDB, IndexB);
}

TArray<UMASkillDefinition*> UMASkillManagerComponent::GetDefinitions(EMAAbilityInputID InputID) const
{
	TArray<UMASkillDefinition*> Result;

	const TArray<TObjectPtr<UMASkillDefinition>>* Definitions = FindSourceDefinitions(InputID);
	if (!Definitions) return Result;

	Result.SetNum(SkillModuleSlotCount);
	const int32 CopyCount = FMath::Min(Definitions->Num(), SkillModuleSlotCount);
	for (int32 Index = 0; Index < CopyCount; ++Index)
	{
		Result[Index] = (*Definitions)[Index];
	}

	return Result;
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

const TArray<TObjectPtr<UMASkillDefinition>>* UMASkillManagerComponent::FindSourceDefinitions(EMAAbilityInputID InputID) const
{
	if (const FMASkillDefinitionStack* SkillStack = FindStack(InputID))
	{
		return &SkillStack->SourceDefinitions;
	}

	// TODO: Keep this only while SkillSlotStacks acts as an editor/test seed path.
	// Runtime systems should seed SkillStacks explicitly, then reads can use SkillStacks only.
	if (const FMASkillSlotStack* SkillSlotStack = FindSkillSlotStack(InputID))
	{
		return &SkillSlotStack->Definitions;
	}

	return nullptr;
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
	Definitions.SetNum(SkillModuleSlotCount);
}

void UMASkillManagerComponent::CopyDefinitionSlots(
	TArray<TObjectPtr<UMASkillDefinition>>& Target,
	const TArray<UMASkillDefinition*>& Source)
{
	Target.Reset();
	Target.SetNum(SkillModuleSlotCount);

	const int32 CopyCount = FMath::Min(Source.Num(), SkillModuleSlotCount);
	for (int32 Index = 0; Index < CopyCount; ++Index)
	{
		Target[Index] = Source[Index];
	}
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
