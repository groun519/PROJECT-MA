#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "MASkillModuleSocketWidget.generated.h"

class UImage;
class UMASkillDefinition;
class UMASkillManagerComponent;
class UMASkillModuleDragVisualWidget;
class UMADescriptionTooltipWidget;

UCLASS()
class P_MA_API UMASkillModuleSocketWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSocket(UMASkillManagerComponent* InSkillManager, EMAAbilityInputID InInputID, int32 InModuleIndex, UMASkillDefinition* InSkillDefinition);

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
	TSubclassOf<UMADescriptionTooltipWidget> TooltipWidgetClass;

private:
	static constexpr float DropHighlightAlpha = 0.15f;
	static constexpr float DraggedSourceRenderOpacity = 0.45f;
	static constexpr float NormalIconScaleMultiplier = 0.65f;
	static constexpr float HighlightedIconScaleMultiplier = 0.75f;
	static const FName HighlightAlphaParameterName;
	static const FName IconScaleMultiplierParameterName;

	void RefreshHoverVisual();
	void RefreshTooltip();
	void SetDraggedSourceVisual(bool bDragged);
	bool IsSelfDragOperation(const UDragDropOperation* Operation) const;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillManagerComponent> SkillManager = nullptr;

	UPROPERTY(Transient)
	EMAAbilityInputID InputID = EMAAbilityInputID::None;

	UPROPERTY(Transient)
	int32 ModuleIndex = INDEX_NONE;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillDefinition> SkillDefinition = nullptr;

	bool bIsHovered = false;
	bool bIsDropTargetHighlighted = false;
};
