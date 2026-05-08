#include "Widget/Skill/MASkillModuleSocketWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widget/MADescriptionTooltipWidget.h"
#include "Widget/Skill/MASkillModuleDragDropOperation.h"
#include "Widget/Skill/MASkillModuleDragVisualWidget.h"

const FName UMASkillModuleSocketWidget::HighlightAlphaParameterName(TEXT("HighlightAlpha"));
const FName UMASkillModuleSocketWidget::IconScaleMultiplierParameterName(TEXT("IconScaleMultiplier"));

void UMASkillModuleSocketWidget::InitializeSocket(
	UMASkillManagerComponent* InSkillManager,
	EMAAbilityInputID InInputID,
	int32 InModuleIndex,
	UMASkillDefinition* InSkillDefinition)
{
	SkillManager = InSkillManager;
	InputID = InInputID;
	ModuleIndex = InModuleIndex;
	SkillDefinition = InSkillDefinition;

	if (!ModuleIconImage) return;

	bIsHovered = false;
	bIsDropTargetHighlighted = false;
	RefreshHoverVisual();
	RefreshTooltip();

	const FMASkillDefinitionIconData* IconData = SkillDefinition ? &SkillDefinition->GetDisplayData().IconData : nullptr;
	if (IconData && IconData->Icon)
	{
		if (UMaterialInstanceDynamic* IconMaterial = ModuleIconImage->GetDynamicMaterial())
		{
			static const FName IconTextureParameterName(TEXT("IconTexture"));
			static const FName IconColorParameterName(TEXT("IconColor"));
			static const FName InnerColorParameterName(TEXT("InnerColor"));
			IconMaterial->SetTextureParameterValue(IconTextureParameterName, IconData->Icon);
			IconMaterial->SetVectorParameterValue(IconColorParameterName, IconData->IconColor);
			IconMaterial->SetVectorParameterValue(InnerColorParameterName, IconData->InnerColor);
		}
	}

	ModuleIconImage->SetVisibility(ESlateVisibility::Visible);
}

void UMASkillModuleSocketWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	bIsHovered = true;
	RefreshHoverVisual();
}

void UMASkillModuleSocketWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	bIsHovered = false;
	RefreshHoverVisual();

	Super::NativeOnMouseLeave(InMouseEvent);
}

FReply UMASkillModuleSocketWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (SkillDefinition && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UMASkillModuleSocketWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	if (!SkillDefinition || ModuleIndex == INDEX_NONE) return;

	UMASkillModuleDragDropOperation* DragOperation = NewObject<UMASkillModuleDragDropOperation>();
	if (!DragOperation) return;

	DragOperation->SourceInputID = InputID;
	DragOperation->SourceModuleIndex = ModuleIndex;
	DragOperation->Pivot = EDragPivot::CenterCenter;
	bIsHovered = false;
	RefreshHoverVisual();
	SetDraggedSourceVisual(true);

	const FMASkillDefinitionIconData& IconData = SkillDefinition->GetDisplayData().IconData;
	if (DragVisualWidgetClass && IconData.Icon)
	{
		UMASkillModuleDragVisualWidget* DragVisual = CreateWidget<UMASkillModuleDragVisualWidget>(this, DragVisualWidgetClass);
		if (DragVisual)
		{
			DragVisual->SetIcon(IconData.Icon, IconData.IconColor);
			DragOperation->DefaultDragVisual = DragVisual;
		}
	}

	OutOperation = DragOperation;
}

void UMASkillModuleSocketWidget::NativeOnDragCancelled(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	SetDraggedSourceVisual(false);
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
}

void UMASkillModuleSocketWidget::NativeOnDragEnter(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	if (Cast<UMASkillModuleDragDropOperation>(InOperation) && !IsSelfDragOperation(InOperation))
	{
		bIsDropTargetHighlighted = true;
		RefreshHoverVisual();
	}
}

void UMASkillModuleSocketWidget::NativeOnDragLeave(
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	bIsDropTargetHighlighted = false;
	RefreshHoverVisual();
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
}

bool UMASkillModuleSocketWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	bIsDropTargetHighlighted = false;
	RefreshHoverVisual();
	SetDraggedSourceVisual(false);

	UMASkillModuleDragDropOperation* DragOperation = Cast<UMASkillModuleDragDropOperation>(InOperation);
	if (!SkillManager || !DragOperation) return false;
	if (IsSelfDragOperation(DragOperation)) return true;

	return SkillManager->RequestSwapDefinitionSlotsBetween(
		DragOperation->SourceInputID,
		DragOperation->SourceModuleIndex,
		InputID,
		ModuleIndex);
}

void UMASkillModuleSocketWidget::NativeDestruct()
{
	bIsHovered = false;
	bIsDropTargetHighlighted = false;
	RefreshHoverVisual();
	SetDraggedSourceVisual(false);
	Super::NativeDestruct();
}

void UMASkillModuleSocketWidget::RefreshHoverVisual()
{
	if (!ModuleIconImage) return;

	if (UMaterialInstanceDynamic* IconMaterial = ModuleIconImage->GetDynamicMaterial())
	{
		const bool bHighlighted = bIsHovered || bIsDropTargetHighlighted;
		IconMaterial->SetScalarParameterValue(HighlightAlphaParameterName, bHighlighted ? DropHighlightAlpha : 0.f);
		IconMaterial->SetScalarParameterValue(IconScaleMultiplierParameterName, bHighlighted ? HighlightedIconScaleMultiplier : NormalIconScaleMultiplier);
	}
}

void UMASkillModuleSocketWidget::RefreshTooltip()
{
	if (!SkillDefinition || !TooltipWidgetClass)
	{
		SetToolTip(nullptr);
		return;
	}

	UMADescriptionTooltipWidget* TooltipWidget = CreateWidget<UMADescriptionTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
	if (!TooltipWidget)
	{
		SetToolTip(nullptr);
		return;
	}

	const FMASkillDefinitionDisplayData& DisplayData = SkillDefinition->GetDisplayData();
	TooltipWidget->SetDescription(DisplayData.DisplayName, DisplayData.Description);
	SetToolTip(TooltipWidget);
}

void UMASkillModuleSocketWidget::SetDraggedSourceVisual(bool bDragged)
{
	SetRenderOpacity(bDragged ? DraggedSourceRenderOpacity : 1.f);
}

bool UMASkillModuleSocketWidget::IsSelfDragOperation(const UDragDropOperation* Operation) const
{
	const UMASkillModuleDragDropOperation* DragOperation = Cast<UMASkillModuleDragDropOperation>(Operation);
	return DragOperation
		&& DragOperation->SourceInputID == InputID
		&& DragOperation->SourceModuleIndex == ModuleIndex;
}
