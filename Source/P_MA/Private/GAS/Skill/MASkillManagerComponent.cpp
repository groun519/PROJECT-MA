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

void UMASkillManagerComponent::ClearDefinitions(EMAAbilityInputID InputID)
{
	if (!CanMutateSkillStacks()) return;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	SkillStack.SourceDefinitions.Reset();
	RebuildSkill(InputID);
}

void UMASkillManagerComponent::SetSingleDefinition(EMAAbilityInputID InputID, UMASkillDefinition* Definition)
{
	if (!CanMutateSkillStacks()) return;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	SkillStack.SourceDefinitions.Reset();
	if (Definition)
	{
		SkillStack.SourceDefinitions.Add(Definition);
	}
	RebuildSkill(InputID);
}

void UMASkillManagerComponent::SetDefinitions(EMAAbilityInputID InputID, const TArray<UMASkillDefinition*>& Definitions)
{
	if (!CanMutateSkillStacks()) return;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	SkillStack.SourceDefinitions.Reset();

	for (UMASkillDefinition* Definition : Definitions)
	{
		if (!Definition) continue;
		SkillStack.SourceDefinitions.Add(Definition);
	}

	RebuildSkill(InputID);
}

bool UMASkillManagerComponent::AddDefinition(EMAAbilityInputID InputID, UMASkillDefinition* Definition)
{
	if (!CanMutateSkillStacks()) return false;
	if (!Definition) return false;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	SkillStack.SourceDefinitions.Add(Definition);
	return RebuildSkill(InputID);
}

bool UMASkillManagerComponent::InsertDefinition(EMAAbilityInputID InputID, int32 InsertIndex, UMASkillDefinition* Definition)
{
	if (!CanMutateSkillStacks()) return false;
	if (!Definition) return false;

	FMASkillDefinitionStack& SkillStack = FindOrAddStack(InputID);
	const int32 ClampedIndex = FMath::Clamp(InsertIndex, 0, SkillStack.SourceDefinitions.Num());
	SkillStack.SourceDefinitions.Insert(Definition, ClampedIndex);
	return RebuildSkill(InputID);
}

bool UMASkillManagerComponent::RemoveDefinitionAt(EMAAbilityInputID InputID, int32 RemoveIndex)
{
	if (!CanMutateSkillStacks()) return false;

	FMASkillDefinitionStack* SkillStack = FindStack(InputID);
	if (!SkillStack || !SkillStack->SourceDefinitions.IsValidIndex(RemoveIndex)) return false;

	SkillStack->SourceDefinitions.RemoveAt(RemoveIndex);
	return RebuildSkill(InputID);
}

bool UMASkillManagerComponent::MoveDefinition(EMAAbilityInputID InputID, int32 FromIndex, int32 ToIndex)
{
	if (!CanMutateSkillStacks()) return false;

	FMASkillDefinitionStack* SkillStack = FindStack(InputID);
	if (!SkillStack || !SkillStack->SourceDefinitions.IsValidIndex(FromIndex)) return false;

	const int32 ClampedToIndex = FMath::Clamp(ToIndex, 0, SkillStack->SourceDefinitions.Num() - 1);
	if (FromIndex == ClampedToIndex) return true;

	UMASkillDefinition* MovedDefinition = SkillStack->SourceDefinitions[FromIndex];
	SkillStack->SourceDefinitions.RemoveAt(FromIndex);
	SkillStack->SourceDefinitions.Insert(MovedDefinition, ClampedToIndex);
	return RebuildSkill(InputID);
}

int32 UMASkillManagerComponent::GetDefinitionCount(EMAAbilityInputID InputID) const
{
	const FMASkillDefinitionStack* SkillStack = FindStack(InputID);
	return SkillStack ? SkillStack->SourceDefinitions.Num() : 0;
}

UMASkillDefinition* UMASkillManagerComponent::GetDefinitionAt(EMAAbilityInputID InputID, int32 Index) const
{
	const FMASkillDefinitionStack* SkillStack = FindStack(InputID);
	if (!SkillStack || !SkillStack->SourceDefinitions.IsValidIndex(Index)) return nullptr;
	return SkillStack->SourceDefinitions[Index];
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
	return SkillStack.AssembledDefinition != nullptr || SkillStack.SourceDefinitions.IsEmpty();
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

	if (!SkillStack.AssembledDefinition)
	{
		return;
	}

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
		return *ExistingStack;
	}

	FMASkillDefinitionStack& NewStack = SkillStacks.AddDefaulted_GetRef();
	NewStack.InputID = InputID;
	if (const FMASkillSlotStack* SkillSlotStack = FindSkillSlotStack(InputID))
	{
		for (UMASkillDefinition* Definition : SkillSlotStack->Definitions)
		{
			if (!Definition) continue;
			NewStack.SourceDefinitions.Add(Definition);
		}
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

TArray<EMAAbilityInputID> UMASkillManagerComponent::GatherUniqueSkillSlotInputIDs() const
{
	TArray<EMAAbilityInputID> UniqueInputIDs;
	TSet<EMAAbilityInputID> SeenInputIDs;

	for (const FMASkillSlotStack& SkillSlotStack : SkillSlotStacks)
	{
		const EMAAbilityInputID InputID = SkillSlotStack.InputID;
		if (InputID == EMAAbilityInputID::None)
		{
			continue;
		}

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
		SkillStack.SourceDefinitions.Reset();
		for (UMASkillDefinition* Definition : ReplicatedSkillSlotStack.Definitions)
		{
			if (!Definition) continue;
			SkillStack.SourceDefinitions.Add(Definition);
		}

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

	ReplicatedSkillSlotStack->Definitions.Reset();
	for (UMASkillDefinition* Definition : SkillStack.SourceDefinitions)
	{
		if (!Definition) continue;
		ReplicatedSkillSlotStack->Definitions.Add(Definition);
	}
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
