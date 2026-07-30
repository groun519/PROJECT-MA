#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"
#include "MASkillModuleSocketWidget.generated.h"

class UImage;
class UTextBlock;
class UMAInventoryComponent;
class UMASkillModule;
class UMASkillManagerComponent;
class UMASkillModuleInstance;
class UMASkillModuleDragVisualWidget;
class UMASkillTooltipWidget;
struct FMAInventoryEntry;
struct FMAItemStack;
struct FMAItemDataRow;
struct FMASkillIconData;

UCLASS()
class P_MA_API UMASkillModuleSocketWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Initialization **/
	void InitializeInventorySlot(UMAInventoryComponent* InInventory, int32 InSlotIndex);
	void InitializeSkillSlot(
		UMASkillManagerComponent* InSkillManager,
		FGameplayTag InSkillSlotTag,
		int32 InSlotIndex);

protected:
	/** UserWidget **/
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeDestruct() override;

	/** Widgets **/
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ModuleIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CooldownOverlayImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> StackText;

	/** Configuration **/
	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<UMASkillModuleDragVisualWidget> DragVisualWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTooltipWidget> TooltipWidgetClass;

private:
	/** Constants **/
	static constexpr float DropHighlightAlpha = 0.15f;
	static constexpr float DraggedSourceRenderOpacity = 0.45f;
	static constexpr float NormalIconScaleMultiplier = 0.65f;
	static constexpr float HighlightedIconScaleMultiplier = 0.75f;
	static constexpr float CooldownVisualUpdateInterval = 0.05f;

	/** Slot Content **/
	void Refresh();
	const FMAInventoryEntry* ResolveInventoryEntry() const;
	UMASkillModuleInstance* ResolveModuleInstance() const;
	const FMAItemDataRow* ResolveItemData(const FMAItemStack& ItemStack) const;

	/** Visuals **/
	void ApplyModuleVisual(const UMASkillModule* Module);
	void ApplyItemVisual(const FMAItemStack& ItemStack);
	void ApplyIconVisual(const FMASkillIconData& IconData, const FLinearColor& FrameColor);
	void RefreshModuleStackText(const UMASkillModuleInstance* ModuleInstance);
	void SetStackText(const FText& Text);
	void RefreshHoverVisual();
	void RefreshTooltip(const FMAInventoryEntry* InventoryEntry);
	void SetDraggedSourceVisual(bool bDragged);

	/** Module State **/
	void ApplyModuleStateVisual(const UMASkillModuleInstance* ModuleInstance);
	void RefreshCooldownDuration(const UMASkillModuleInstance* ModuleInstance);
	void RefreshCooldownVisual();
	void ClearCooldownVisualTimer();
	void BindModuleState(UMASkillModuleInstance* ModuleInstance);
	void UnbindModuleState();
	void HandleModuleStateChanged();

	/** Interaction **/
	UFUNCTION()
	void HandleDragCompleted(UDragDropOperation* Operation);

	/** Context **/
	TWeakObjectPtr<UMAInventoryComponent> Inventory;
	TWeakObjectPtr<UMASkillManagerComponent> SkillManager;
	FGameplayTag SkillSlotTag;
	int32 SlotIndex = INDEX_NONE;

	/** Runtime State **/
	UPROPERTY(Transient)
	TObjectPtr<UMASkillModule> CachedModule = nullptr;

	int32 DisplayedEntryId = INDEX_NONE;
	TWeakObjectPtr<UMASkillModuleInstance> BoundModuleInstance;
	FDelegateHandle ModuleStateChangedHandle;
	FTimerHandle CooldownVisualTimerHandle;
	float CooldownDurationSeconds = 0.f;
	bool bIsHovered = false;
	bool bIsDropTargetHighlighted = false;
};
