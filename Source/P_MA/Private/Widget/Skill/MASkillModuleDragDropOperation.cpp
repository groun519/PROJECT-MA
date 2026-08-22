#include "Widget/Skill/MASkillModuleDragDropOperation.h"

#include "GAS/Skill/MASkillManagerComponent.h"
#include "Inventory/MAInventoryComponent.h"

/** Source **/
bool UMASkillModuleDragDropOperation::SetSource(
	UMAInventoryComponent* InInventory,
	const FMAInventoryEntry& InEntry)
{
	if (!InInventory || (!InEntry.IsModuleInstance() && !InEntry.IsStack())) return false;

	SourceInventory = InInventory;
	SourceInventoryEntryId = InEntry.EntryId;
	SourceInventoryEntryKind = InEntry.Kind;
	SourceSkillManager.Reset();
	SourceSkillSlotTag = FGameplayTag();
	SourceSkillModuleIndex = INDEX_NONE;
	return true;
}

bool UMASkillModuleDragDropOperation::SetSource(
	UMASkillManagerComponent* InSkillManager,
	const FGameplayTag InSkillSlotTag,
	const int32 InModuleIndex)
{
	if (!InSkillManager || !InSkillSlotTag.IsValid() || InModuleIndex == INDEX_NONE) return false;

	SourceInventory.Reset();
	SourceInventoryEntryId = INDEX_NONE;
	SourceInventoryEntryKind = EMAInventoryEntryKind::Empty;
	SourceSkillManager = InSkillManager;
	SourceSkillSlotTag = InSkillSlotTag;
	SourceSkillModuleIndex = InModuleIndex;
	return true;
}

/** Drop Target **/
bool UMASkillModuleDragDropOperation::CanDropOn(
	UMAInventoryComponent* TargetInventory,
	const int32 TargetSlotIndex) const
{
	if (!TargetInventory) return false;

	if (IsFromInventory())
	{
		const FMAInventoryEntry* TargetEntry = TargetInventory->GetEntryAt(TargetSlotIndex);
		return SourceInventory.Get() == TargetInventory
			&& TargetEntry
			&& !IsSourceInventoryEntry(TargetInventory, TargetEntry->EntryId);
	}

	if (!IsFromSkillSlot()
		|| SourceSkillManager->GetOwner() != TargetInventory->GetOwner())
	{
		return false;
	}

	const FMAInventoryEntry* TargetEntry = TargetInventory->GetEntryAt(TargetSlotIndex);
	return TargetEntry && (TargetEntry->IsEmpty() || TargetEntry->IsModuleInstance());
}

bool UMASkillModuleDragDropOperation::CanDropOn(
	UMASkillManagerComponent* TargetSkillManager,
	const FGameplayTag TargetSkillSlotTag,
	const int32 TargetModuleIndex) const
{
	if (!TargetSkillManager) return false;

	if (IsFromInventory())
	{
		return SourceInventoryEntryKind == EMAInventoryEntryKind::ModuleInstance
			&& SourceInventory->GetOwner() == TargetSkillManager->GetOwner();
	}

	return IsFromSkillSlot()
		&& SourceSkillManager.Get() == TargetSkillManager
		&& !IsSourceSkillSlot(
			TargetSkillManager,
			TargetSkillSlotTag,
			TargetModuleIndex);
}

bool UMASkillModuleDragDropOperation::TryDropOn(
	UMAInventoryComponent* TargetInventory,
	const int32 TargetSlotIndex) const
{
	if (!TargetInventory) return false;
	const FMAInventoryEntry* TargetEntry = TargetInventory->GetEntryAt(TargetSlotIndex);
	if (TargetEntry && IsSourceInventoryEntry(TargetInventory, TargetEntry->EntryId)) return true;
	if (!CanDropOn(TargetInventory, TargetSlotIndex)) return false;

	if (IsFromInventory())
	{
		UMAInventoryComponent* Inventory = SourceInventory.Get();
		return Inventory->RequestMoveEntry(SourceInventoryEntryId, TargetSlotIndex);
	}

	return TargetInventory->RequestMoveSkillModuleToInventory(
		SourceSkillSlotTag,
		SourceSkillModuleIndex,
		TargetSlotIndex);
}

bool UMASkillModuleDragDropOperation::TryDropOn(
	UMASkillManagerComponent* TargetSkillManager,
	const FGameplayTag TargetSkillSlotTag,
	const int32 TargetModuleIndex) const
{
	if (!TargetSkillManager) return false;
	if (IsSourceSkillSlot(TargetSkillManager, TargetSkillSlotTag, TargetModuleIndex)) return true;
	if (!CanDropOn(TargetSkillManager, TargetSkillSlotTag, TargetModuleIndex)) return false;

	if (IsFromInventory())
	{
		UMAInventoryComponent* Inventory = SourceInventory.Get();
		return Inventory->RequestEquipModule(
			SourceInventoryEntryId,
			TargetSkillSlotTag,
			TargetModuleIndex);
	}

	UMASkillManagerComponent* SkillManager = SourceSkillManager.Get();
	return SkillManager->RequestSwapModuleSlotsBetween(
		SourceSkillSlotTag,
		SourceSkillModuleIndex,
		TargetSkillSlotTag,
		TargetModuleIndex);
}

/** Source State **/
bool UMASkillModuleDragDropOperation::IsFromInventory() const
{
	return SourceInventory.IsValid()
		&& SourceInventoryEntryId != INDEX_NONE
		&& SourceInventoryEntryKind != EMAInventoryEntryKind::Empty;
}

bool UMASkillModuleDragDropOperation::IsFromSkillSlot() const
{
	return SourceSkillManager.IsValid()
		&& SourceSkillSlotTag.IsValid()
		&& SourceSkillModuleIndex != INDEX_NONE;
}

bool UMASkillModuleDragDropOperation::IsSourceInventoryEntry(
	const UMAInventoryComponent* Inventory,
	const int32 EntryId) const
{
	return IsFromInventory()
		&& SourceInventory.Get() == Inventory
		&& SourceInventoryEntryId == EntryId;
}

bool UMASkillModuleDragDropOperation::IsSourceSkillSlot(
	const UMASkillManagerComponent* SkillManager,
	const FGameplayTag SkillSlotTag,
	const int32 ModuleIndex) const
{
	return IsFromSkillSlot()
		&& SourceSkillManager.Get() == SkillManager
		&& SourceSkillSlotTag == SkillSlotTag
		&& SourceSkillModuleIndex == ModuleIndex;
}
