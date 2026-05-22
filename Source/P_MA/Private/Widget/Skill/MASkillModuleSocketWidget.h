#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASkillModuleSocketWidget.generated.h"

class UActorComponent;
class UImage;
class UMASkillDefinition;
class UMASkillModuleInstance;
class UMASkillModuleDragVisualWidget;
class UMASkillTooltipWidget;

UCLASS()
class P_MA_API UMASkillModuleSocketWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSocket(
		UActorComponent* InSlotOwner,
		const TArray<TObjectPtr<UMASkillModuleInstance>>* InSlotArray,
		int32 InSlotIndex);
	void Refresh();

protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ModuleIconImage;

	UPROPERTY(EditDefaultsOnly, Category="Skill")
	TSubclassOf<UMASkillModuleDragVisualWidget> DragVisualWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Tooltip")
	TSubclassOf<UMASkillTooltipWidget> TooltipWidgetClass;

private:
	static constexpr float DropHighlightAlpha = 0.15f;
	static constexpr float DraggedSourceRenderOpacity = 0.45f;
	static constexpr float NormalIconScaleMultiplier = 0.65f;
	static constexpr float HighlightedIconScaleMultiplier = 0.75f;
	UMASkillDefinition* ResolveDefinition() const;
	bool IsValidSlot() const;
	void ApplyDefinitionVisual(const UMASkillDefinition* Definition);
	void RefreshHoverVisual();
	void RefreshTooltip();
	void SetDraggedSourceVisual(bool bDragged);
	bool HandleDropFrom(
		UActorComponent* SourceOwner,
		const TArray<TObjectPtr<UMASkillModuleInstance>>* SourceSlots,
		int32 SourceIndex);
	bool IsSelfDragOperation(const UDragDropOperation* Operation) const;

	TWeakObjectPtr<UActorComponent> SlotOwner;
	const TArray<TObjectPtr<UMASkillModuleInstance>>* SlotArray = nullptr;
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillDefinition> CachedDefinition = nullptr;

	bool bIsHovered = false;
	bool bIsDropTargetHighlighted = false;
};
