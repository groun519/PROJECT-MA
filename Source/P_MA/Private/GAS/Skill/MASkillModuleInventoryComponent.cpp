#include "GAS/Skill/MASkillModuleInventoryComponent.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "Net/UnrealNetwork.h"

UMASkillModuleInventoryComponent::UMASkillModuleInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMASkillModuleInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureSlotCount();
}

void UMASkillModuleInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMASkillModuleInventoryComponent, Entries, COND_OwnerOnly);
}

bool UMASkillModuleInventoryComponent::AddModule(UMASkillDefinition* Definition)
{
	if (!CanMutateInventory()) return false;
	if (!Definition) return false;

	EnsureSlotCount();
	const int32 EmptyIndex = Entries.IndexOfByPredicate([](const UMASkillDefinition* Candidate)
	{
		return Candidate == nullptr;
	});
	if (EmptyIndex == INDEX_NONE) return false;

	Entries[EmptyIndex] = Definition;
	OnInventoryChanged.Broadcast();
	return true;
}

bool UMASkillModuleInventoryComponent::RequestGrantModule(UMASkillDefinition* Definition)
{
	if (!Definition) return false;

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	if (OwnerActor->HasAuthority())
	{
		return AddModule(Definition);
	}

	ServerGrantModule(Definition);
	return true;
}

bool UMASkillModuleInventoryComponent::RequestMoveModuleSlot(
	const TArray<TObjectPtr<UMASkillDefinition>>* SourceSlots,
	int32 SourceIndex,
	UActorComponent* TargetOwner,
	const TArray<TObjectPtr<UMASkillDefinition>>* TargetSlots,
	int32 TargetIndex)
{
	if (SourceSlots != &Entries) return false;
	if (!IsValidSlotIndex(SourceIndex) || !Entries[SourceIndex]) return false;
	if (!TargetOwner || !TargetSlots || TargetIndex == INDEX_NONE) return false;

	if (TargetOwner == this && TargetSlots == &Entries)
	{
		const AActor* OwnerActor = GetOwner();
		if (!OwnerActor) return false;

		if (OwnerActor->HasAuthority())
		{
			return SwapInventorySlots(SourceIndex, TargetIndex);
		}

		ServerSwapInventorySlots(SourceIndex, TargetIndex);
		return true;
	}

	UMASkillManagerComponent* TargetSkillManager = Cast<UMASkillManagerComponent>(TargetOwner);
	if (!TargetSkillManager) return false;

	EMAAbilityInputID TargetInputID = EMAAbilityInputID::None;
	if (!TargetSkillManager->FindInputIDForDefinitionSlots(TargetSlots, TargetInputID)) return false;

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	if (OwnerActor->HasAuthority())
	{
		return EquipInventorySlotToSkillSlot(SourceIndex, TargetInputID, TargetIndex);
	}

	ServerEquipInventorySlotToSkillSlot(SourceIndex, TargetInputID, TargetIndex);
	return true;
}

bool UMASkillModuleInventoryComponent::RequestMoveSkillSlotToInventorySlot(
	EMAAbilityInputID InputID,
	int32 ModuleIndex,
	int32 TargetSlotIndex)
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;

	if (OwnerActor->HasAuthority())
	{
		return MoveSkillSlotToInventorySlot(InputID, ModuleIndex, TargetSlotIndex);
	}

	ServerMoveSkillSlotToInventorySlot(InputID, ModuleIndex, TargetSlotIndex);
	return true;
}

bool UMASkillModuleInventoryComponent::EquipInventorySlotToSkillSlot(
	int32 SourceSlotIndex,
	EMAAbilityInputID InputID,
	int32 ModuleIndex)
{
	if (!CanMutateInventory()) return false;

	EnsureSlotCount();
	if (!IsValidSlotIndex(SourceSlotIndex) || !Entries[SourceSlotIndex]) return false;

	UMASkillManagerComponent* SkillManager = GetOwner() ? GetOwner()->FindComponentByClass<UMASkillManagerComponent>() : nullptr;
	if (!SkillManager) return false;

	UMASkillDefinition* PreviousDefinition = nullptr;
	UMASkillDefinition* NewDefinition = Entries[SourceSlotIndex];
	if (!SkillManager->ReplaceDefinitionAt(InputID, ModuleIndex, NewDefinition, PreviousDefinition)) return false;

	Entries[SourceSlotIndex] = PreviousDefinition;
	OnInventoryChanged.Broadcast();
	return true;
}

void UMASkillModuleInventoryComponent::ServerGrantModule_Implementation(UMASkillDefinition* Definition)
{
	AddModule(Definition);
}

void UMASkillModuleInventoryComponent::ServerEquipInventorySlotToSkillSlot_Implementation(
	int32 SourceSlotIndex,
	EMAAbilityInputID InputID,
	int32 ModuleIndex)
{
	EquipInventorySlotToSkillSlot(SourceSlotIndex, InputID, ModuleIndex);
}

void UMASkillModuleInventoryComponent::ServerSwapInventorySlots_Implementation(int32 SourceSlotIndex, int32 TargetSlotIndex)
{
	SwapInventorySlots(SourceSlotIndex, TargetSlotIndex);
}

void UMASkillModuleInventoryComponent::ServerMoveSkillSlotToInventorySlot_Implementation(
	EMAAbilityInputID InputID,
	int32 ModuleIndex,
	int32 TargetSlotIndex)
{
	MoveSkillSlotToInventorySlot(InputID, ModuleIndex, TargetSlotIndex);
}

const TArray<TObjectPtr<UMASkillDefinition>>* UMASkillModuleInventoryComponent::GetModuleSlotsForUI()
{
	EnsureSlotCount();
	return &Entries;
}

bool UMASkillModuleInventoryComponent::CanMutateInventory() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

bool UMASkillModuleInventoryComponent::SwapInventorySlots(int32 SourceSlotIndex, int32 TargetSlotIndex)
{
	if (!CanMutateInventory()) return false;
	if (SourceSlotIndex == TargetSlotIndex) return true;

	EnsureSlotCount();
	if (!IsValidSlotIndex(SourceSlotIndex) || !IsValidSlotIndex(TargetSlotIndex)) return false;
	if (!Entries[SourceSlotIndex]) return false;

	Swap(Entries[SourceSlotIndex], Entries[TargetSlotIndex]);
	OnInventoryChanged.Broadcast();
	return true;
}

bool UMASkillModuleInventoryComponent::MoveSkillSlotToInventorySlot(
	EMAAbilityInputID InputID,
	int32 ModuleIndex,
	int32 TargetSlotIndex)
{
	if (!CanMutateInventory()) return false;

	EnsureSlotCount();
	if (!IsValidSlotIndex(TargetSlotIndex)) return false;

	UMASkillManagerComponent* SkillManager = GetOwner() ? GetOwner()->FindComponentByClass<UMASkillManagerComponent>() : nullptr;
	if (!SkillManager) return false;

	UMASkillDefinition* PreviousSkillDefinition = nullptr;
	if (!SkillManager->ReplaceDefinitionAt(InputID, ModuleIndex, Entries[TargetSlotIndex], PreviousSkillDefinition)) return false;
	if (!PreviousSkillDefinition)
	{
		UMASkillDefinition* IgnoredDefinition = nullptr;
		SkillManager->ReplaceDefinitionAt(InputID, ModuleIndex, nullptr, IgnoredDefinition);
		return false;
	}

	Entries[TargetSlotIndex] = PreviousSkillDefinition;
	OnInventoryChanged.Broadcast();
	return true;
}

bool UMASkillModuleInventoryComponent::IsValidSlotIndex(int32 SlotIndex) const
{
	return SlotIndex >= 0 && SlotIndex < MaxSlotCount && Entries.IsValidIndex(SlotIndex);
}

void UMASkillModuleInventoryComponent::EnsureSlotCount()
{
	const int32 TargetCount = FMath::Max(0, MaxSlotCount);
	if (Entries.Num() == TargetCount) return;

	// UI socket widgets may hold a direct pointer to this TArray object. Keep the array object stable,
	// and only resize before widgets are bound. Runtime module moves should swap/replace values only.
	ensureMsgf(
		!OnInventoryChanged.IsBound(),
		TEXT("Do not resize module inventory slots while UI widgets may hold direct slot array pointers."));
	Entries.SetNum(TargetCount);
}

void UMASkillModuleInventoryComponent::OnRep_Entries()
{
	OnInventoryChanged.Broadcast();
}
