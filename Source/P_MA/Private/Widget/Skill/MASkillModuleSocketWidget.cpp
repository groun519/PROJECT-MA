#include "Widget/Skill/MASkillModuleSocketWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Widget/Skill/MASkillModuleDragDropOperation.h"
#include "Widget/Skill/MASkillModuleDragVisualWidget.h"

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
	DragOperation->IconTexture = SkillDefinition->GetDisplayData().IconData.Icon;
	DragOperation->IconColor = SkillDefinition->GetDisplayData().IconData.IconColor;
	DragOperation->Pivot = EDragPivot::CenterCenter;

	if (DragVisualWidgetClass && DragOperation->IconTexture)
	{
		UMASkillModuleDragVisualWidget* DragVisual = CreateWidget<UMASkillModuleDragVisualWidget>(this, DragVisualWidgetClass);
		if (DragVisual)
		{
			DragVisual->SetIcon(DragOperation->IconTexture, DragOperation->IconColor);
			DragOperation->DefaultDragVisual = DragVisual;
		}
	}

	OutOperation = DragOperation;
}

bool UMASkillModuleSocketWidget::NativeOnDrop(
	const FGeometry& InGeometry,
	const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UMASkillModuleDragDropOperation* DragOperation = Cast<UMASkillModuleDragDropOperation>(InOperation);
	if (!SkillManager || !DragOperation) return false;
	if (DragOperation->SourceInputID == InputID
		&& DragOperation->SourceModuleIndex == ModuleIndex) return true;

	return SkillManager->RequestSwapDefinitionSlotsBetween(
		DragOperation->SourceInputID,
		DragOperation->SourceModuleIndex,
		InputID,
		ModuleIndex);
}
