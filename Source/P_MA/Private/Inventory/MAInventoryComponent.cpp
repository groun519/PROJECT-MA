#include "Inventory/MAInventoryComponent.h"

#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "Item/Data/MAModuleItemData.h"
#include "Net/UnrealNetwork.h"
#include "Setting/MAGameSettings.h"

UMAInventoryComponent::UMAInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMAInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureSlotCount();
	RefreshEntryModuleStates();
}

void UMAInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UMAInventoryComponent, Entries, COND_OwnerOnly);
}

/** Module **/
bool UMAInventoryComponent::RequestGrantModule(UMASkillModule* Module)
{
	if (!Module) return false;

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;
	if (OwnerActor->HasAuthority()) return AddModule(Module);

	ServerGrantModule(Module);
	return true;
}

/** Item **/
bool UMAInventoryComponent::RequestGrantItem(const FName ItemRowName, const int32 Count)
{
	if (ItemRowName.IsNone() || Count <= 0) return false;

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;
	if (OwnerActor->HasAuthority()) return AddItem(ItemRowName, Count);

	ServerGrantItem(ItemRowName, Count);
	return true;
}

bool UMAInventoryComponent::RequestConsumeItem(const int32 EntryId, const int32 Count)
{
	if (EntryId == INDEX_NONE || Count <= 0) return false;

	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;
	if (OwnerActor->HasAuthority()) return ConsumeItem(EntryId, Count);

	ServerConsumeItem(EntryId, Count);
	return true;
}

/** Entry Transfer **/
bool UMAInventoryComponent::RequestMoveEntry(const int32 EntryId, const int32 TargetSlotIndex)
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;
	if (OwnerActor->HasAuthority()) return MoveEntry(EntryId, TargetSlotIndex);

	ServerMoveEntry(EntryId, TargetSlotIndex);
	return true;
}

bool UMAInventoryComponent::RequestEquipModule(
	const int32 EntryId,
	const FGameplayTag SkillSlotTag,
	const int32 ModuleIndex)
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;
	if (OwnerActor->HasAuthority()) return EquipModule(EntryId, SkillSlotTag, ModuleIndex);

	ServerEquipModule(EntryId, SkillSlotTag, ModuleIndex);
	return true;
}

bool UMAInventoryComponent::RequestMoveSkillModuleToInventory(
	const FGameplayTag SkillSlotTag,
	const int32 ModuleIndex,
	const int32 TargetSlotIndex)
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor) return false;
	if (OwnerActor->HasAuthority())
	{
		return MoveSkillModuleToInventory(SkillSlotTag, ModuleIndex, TargetSlotIndex);
	}

	ServerMoveSkillModuleToInventory(SkillSlotTag, ModuleIndex, TargetSlotIndex);
	return true;
}

/** Query **/
const FMAInventoryEntry* UMAInventoryComponent::GetEntryAt(const int32 SlotIndex) const
{
	return Entries.IsValidIndex(SlotIndex) ? &Entries[SlotIndex] : nullptr;
}

UMASkillModuleInstance* UMAInventoryComponent::GetModuleAt(const int32 SlotIndex) const
{
	const FMAInventoryEntry* Entry = GetEntryAt(SlotIndex);
	return Entry && Entry->IsModule() ? Entry->ModuleInstance : nullptr;
}

/** Module **/
bool UMAInventoryComponent::AddModule(UMASkillModule* Module)
{
	if (!CanMutateInventory() || !Module) return false;

	EnsureSlotCount();
	const int32 EmptySlotIndex = Entries.IndexOfByPredicate([](const FMAInventoryEntry& Entry)
	{
		return Entry.IsEmpty();
	});
	if (EmptySlotIndex == INDEX_NONE) return false;

	UMASkillManagerComponent* SkillManager = GetOwner()->FindComponentByClass<UMASkillManagerComponent>();
	if (!SkillManager) return false;

	UMASkillModuleInstance* ModuleInstance = SkillManager->CreateModuleInstance(Module);
	if (!ModuleInstance) return false;

	Entries[EmptySlotIndex].SetModule(AllocateEntryId(), ModuleInstance);
	NotifyInventoryChanged();
	return true;
}

/** Item **/
bool UMAInventoryComponent::AddItem(const FName ItemRowName, const int32 Count)
{
	if (!CanMutateInventory() || ItemRowName.IsNone() || Count <= 0) return false;

	const UDataTable* ItemDataTable = UMAGameSettings::Get()->GetModuleItemDataTable();
	if (!ItemDataTable
		|| !ItemDataTable->FindRow<FMAModuleItemDataRow>(ItemRowName, TEXT("AddItem")))
	{
		return false;
	}

	EnsureSlotCount();
	FMAInventoryEntry* TargetEntry = Entries.FindByPredicate([ItemRowName](const FMAInventoryEntry& Entry)
	{
		return Entry.IsItem() && Entry.ItemStack.ItemRowName == ItemRowName;
	});
	if (TargetEntry)
	{
		if (TargetEntry->ItemStack.Count > MAX_int32 - Count) return false;
		TargetEntry->ItemStack.Count += Count;
	}
	else
	{
		TargetEntry = Entries.FindByPredicate([](const FMAInventoryEntry& Entry)
		{
			return Entry.IsEmpty();
		});
		if (!TargetEntry) return false;

		TargetEntry->SetItem(AllocateEntryId(), ItemRowName, Count);
	}

	NotifyInventoryChanged();
	return true;
}

bool UMAInventoryComponent::ConsumeItem(const int32 EntryId, const int32 Count)
{
	if (!CanMutateInventory() || EntryId == INDEX_NONE || Count <= 0) return false;

	const int32 SlotIndex = FindEntrySlot(EntryId);
	if (!Entries.IsValidIndex(SlotIndex)) return false;

	FMAInventoryEntry& Entry = Entries[SlotIndex];
	if (!Entry.IsItem() || Entry.ItemStack.Count < Count) return false;

	if (Entry.ItemStack.Count == Count)
	{
		Entry.Reset();
	}
	else
	{
		Entry.ItemStack.Count -= Count;
	}

	NotifyInventoryChanged();
	return true;
}

/** Entry Transfer **/
bool UMAInventoryComponent::MoveEntry(const int32 EntryId, const int32 TargetSlotIndex)
{
	if (!CanMutateInventory()) return false;

	EnsureSlotCount();
	const int32 SourceSlotIndex = FindEntrySlot(EntryId);
	if (!Entries.IsValidIndex(SourceSlotIndex) || !Entries.IsValidIndex(TargetSlotIndex)) return false;
	if (SourceSlotIndex == TargetSlotIndex) return true;

	Swap(Entries[SourceSlotIndex], Entries[TargetSlotIndex]);
	NotifyInventoryChanged();
	return true;
}

bool UMAInventoryComponent::EquipModule(
	const int32 EntryId,
	const FGameplayTag SkillSlotTag,
	const int32 ModuleIndex)
{
	if (!CanMutateInventory()) return false;

	EnsureSlotCount();
	const int32 SourceSlotIndex = FindEntrySlot(EntryId);
	if (!Entries.IsValidIndex(SourceSlotIndex)) return false;

	FMAInventoryEntry& SourceEntry = Entries[SourceSlotIndex];
	if (!SourceEntry.IsModule()) return false;

	UMASkillManagerComponent* SkillManager = GetOwner()->FindComponentByClass<UMASkillManagerComponent>();
	if (!SkillManager) return false;

	UMASkillModuleInstance* PreviousModuleInstance = nullptr;
	if (!SkillManager->ReplaceModuleInstanceAt(
		SkillSlotTag,
		ModuleIndex,
		SourceEntry.ModuleInstance,
		PreviousModuleInstance))
	{
		return false;
	}

	SourceEntry.Reset();
	if (PreviousModuleInstance)
	{
		PreviousModuleInstance->SetActive(true);
		SourceEntry.SetModule(AllocateEntryId(), PreviousModuleInstance);
	}
	NotifyInventoryChanged();
	return true;
}

bool UMAInventoryComponent::MoveSkillModuleToInventory(
	const FGameplayTag SkillSlotTag,
	const int32 ModuleIndex,
	const int32 TargetSlotIndex)
{
	if (!CanMutateInventory()) return false;

	EnsureSlotCount();
	if (!Entries.IsValidIndex(TargetSlotIndex)) return false;

	FMAInventoryEntry& TargetEntry = Entries[TargetSlotIndex];
	if (!TargetEntry.IsEmpty() && !TargetEntry.IsModule()) return false;

	UMASkillManagerComponent* SkillManager = GetOwner()->FindComponentByClass<UMASkillManagerComponent>();
	if (!SkillManager || !SkillManager->GetModuleInstanceAt(SkillSlotTag, ModuleIndex)) return false;

	UMASkillModuleInstance* PreviousSkillModuleInstance = nullptr;
	if (!SkillManager->ReplaceModuleInstanceAt(
		SkillSlotTag,
		ModuleIndex,
		TargetEntry.IsModule() ? TargetEntry.ModuleInstance.Get() : nullptr,
		PreviousSkillModuleInstance))
	{
		return false;
	}
	check(PreviousSkillModuleInstance);

	PreviousSkillModuleInstance->SetActive(true);
	TargetEntry.SetModule(AllocateEntryId(), PreviousSkillModuleInstance);
	NotifyInventoryChanged();
	return true;
}

/** Internal **/
bool UMAInventoryComponent::CanMutateInventory() const
{
	const AActor* OwnerActor = GetOwner();
	return OwnerActor && OwnerActor->HasAuthority();
}

int32 UMAInventoryComponent::AllocateEntryId()
{
	return NextEntryId++;
}

int32 UMAInventoryComponent::FindEntrySlot(const int32 EntryId) const
{
	if (EntryId == INDEX_NONE) return INDEX_NONE;

	return Entries.IndexOfByPredicate([EntryId](const FMAInventoryEntry& Entry)
	{
		return Entry.EntryId == EntryId && !Entry.IsEmpty();
	});
}

void UMAInventoryComponent::EnsureSlotCount()
{
	const int32 TargetCount = FMath::Max(0, MaxSlotCount);
	if (Entries.Num() == TargetCount) return;

	Entries.SetNum(TargetCount);
}

void UMAInventoryComponent::RefreshEntryModuleStates()
{
	for (const FMAInventoryEntry& Entry : Entries)
	{
		if (!Entry.IsModule()) continue;

		Entry.ModuleInstance->SetInSkillSlot(false);
		Entry.ModuleInstance->SetActive(true);
	}
}

void UMAInventoryComponent::NotifyInventoryChanged()
{
	OnInventoryChanged.Broadcast();
}

/** Replication **/
void UMAInventoryComponent::ServerGrantModule_Implementation(UMASkillModule* Module)
{
	AddModule(Module);
}

void UMAInventoryComponent::ServerGrantItem_Implementation(
	const FName ItemRowName,
	const int32 Count)
{
	AddItem(ItemRowName, Count);
}

void UMAInventoryComponent::ServerConsumeItem_Implementation(
	const int32 EntryId,
	const int32 Count)
{
	ConsumeItem(EntryId, Count);
}

void UMAInventoryComponent::ServerMoveEntry_Implementation(
	const int32 EntryId,
	const int32 TargetSlotIndex)
{
	MoveEntry(EntryId, TargetSlotIndex);
}

void UMAInventoryComponent::ServerEquipModule_Implementation(
	const int32 EntryId,
	const FGameplayTag SkillSlotTag,
	const int32 ModuleIndex)
{
	EquipModule(EntryId, SkillSlotTag, ModuleIndex);
}

void UMAInventoryComponent::ServerMoveSkillModuleToInventory_Implementation(
	const FGameplayTag SkillSlotTag,
	const int32 ModuleIndex,
	const int32 TargetSlotIndex)
{
	MoveSkillModuleToInventory(SkillSlotTag, ModuleIndex, TargetSlotIndex);
}

void UMAInventoryComponent::OnRep_Entries()
{
	RefreshEntryModuleStates();
	OnInventoryChanged.Broadcast();
}
