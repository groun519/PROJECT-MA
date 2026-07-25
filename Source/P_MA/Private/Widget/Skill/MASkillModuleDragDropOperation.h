#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "GameplayTagContainer.h"
#include "Inventory/MAInventoryTypes.h"
#include "MASkillModuleDragDropOperation.generated.h"

class UMAInventoryComponent;
class UMASkillManagerComponent;

UCLASS()
class P_MA_API UMASkillModuleDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** Source **/
	bool SetSource(UMAInventoryComponent* InInventory, const FMAInventoryEntry& InEntry);
	bool SetSource(
		UMASkillManagerComponent* InSkillManager,
		FGameplayTag InSkillSlotTag,
		int32 InModuleIndex);

	/** Drop Target **/
	bool CanDropOn(
		UMAInventoryComponent* TargetInventory,
		int32 TargetSlotIndex) const;
	bool CanDropOn(
		UMASkillManagerComponent* TargetSkillManager,
		FGameplayTag TargetSkillSlotTag,
		int32 TargetModuleIndex) const;
	bool TryDropOn(
		UMAInventoryComponent* TargetInventory,
		int32 TargetSlotIndex) const;
	bool TryDropOn(
		UMASkillManagerComponent* TargetSkillManager,
		FGameplayTag TargetSkillSlotTag,
		int32 TargetModuleIndex) const;

private:
	/** Source State **/
	bool IsFromInventory() const;
	bool IsFromSkillSlot() const;
	bool IsSourceInventoryEntry(
		const UMAInventoryComponent* Inventory,
		int32 EntryId) const;
	bool IsSourceSkillSlot(
		const UMASkillManagerComponent* SkillManager,
		FGameplayTag SkillSlotTag,
		int32 ModuleIndex) const;

	TWeakObjectPtr<UMAInventoryComponent> SourceInventory;
	int32 SourceInventoryEntryId = INDEX_NONE;
	EMAInventoryEntryKind SourceInventoryEntryKind = EMAInventoryEntryKind::Empty;

	TWeakObjectPtr<UMASkillManagerComponent> SourceSkillManager;
	FGameplayTag SourceSkillSlotTag;
	int32 SourceSkillModuleIndex = INDEX_NONE;
};
