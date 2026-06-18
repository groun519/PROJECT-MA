#include "Widget/Skill/MASkillModuleSocketWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/MASkillGenericDataAsset.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Skill/MASkillModuleDragDropOperation.h"
#include "Widget/Skill/MASkillModuleDragVisualWidget.h"
#include "Widget/Skill/MASkillTooltipWidget.h"

void UMASkillModuleSocketWidget::InitializeSocket(
	UActorComponent* InSlotOwner,
	const TArray<TObjectPtr<UMASkillModuleInstance>>* InSlotArray,
	int32 InSlotIndex)
{
	SlotOwner = InSlotOwner;
	SlotArray = InSlotArray;
	SlotIndex = InSlotIndex;

	Refresh();
}

void UMASkillModuleSocketWidget::Refresh()
{
	if (!ModuleIconImage) return;

	SetDraggedSourceVisual(false);
	bIsHovered = false;
	bIsDropTargetHighlighted = false;
	RefreshHoverVisual();

	UMASkillModuleInstance* ModuleInstance = ResolveModuleInstance();
	CachedDefinition = ModuleInstance ? ModuleInstance->GetDefinition() : nullptr;
	ApplyDefinitionVisual(CachedDefinition);
	ApplyActivationVisual(ModuleInstance);
	RefreshTooltip();
}

UMASkillModuleInstance* UMASkillModuleSocketWidget::ResolveModuleInstance() const
{
	return IsValidSlot() ? (*SlotArray)[SlotIndex] : nullptr;
}

UMASkillDefinition* UMASkillModuleSocketWidget::ResolveDefinition() const
{
	UMASkillModuleInstance* ModuleInstance = ResolveModuleInstance();
	return ModuleInstance ? ModuleInstance->GetDefinition() : nullptr;
}

const UDataTable* UMASkillModuleSocketWidget::ResolveWarningTextDataTable() const
{
	const UMASkillGenericDataAsset* GenericSkillDataAsset = UMAGameSettings::Get()->GetDefaultSkillGenericDataAsset();
	return GenericSkillDataAsset ? GenericSkillDataAsset->GetWarningTextDataTable() : nullptr;
}

bool UMASkillModuleSocketWidget::IsValidSlot() const
{
	return SlotOwner.IsValid() && SlotArray && SlotArray->IsValidIndex(SlotIndex);
}

void UMASkillModuleSocketWidget::ApplyDefinitionVisual(const UMASkillDefinition* Definition)
{
	if (!ModuleIconImage) return;

	const UMAModuleQualityData* ModuleQualityData = UMAGameSettings::Get()->GetModuleQualityData();
	const FMASkillDefinitionIconData IconData = Definition
		? Definition->ResolveIconData(ModuleQualityData)
		: FMASkillDefinitionIconData();
	const bool bHasIcon = IconData.Icon != nullptr;
	if (UMaterialInstanceDynamic* IconMaterial = ModuleIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_SubIconTexture, nullptr);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, Definition ? Definition->ResolveFrameColor(ModuleQualityData) : FLinearColor::White);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseIcon, bHasIcon ? 1.f : 0.f);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_UseSubIcon, 0.f);
	}
	else if (bHasIcon)
	{
		ModuleIconImage->SetBrushFromTexture(IconData.Icon);
	}
	else
	{
		ModuleIconImage->SetBrush(FSlateBrush());
	}

	ModuleIconImage->SetVisibility(ESlateVisibility::Visible);
}

void UMASkillModuleSocketWidget::ApplyActivationVisual(const UMASkillModuleInstance* ModuleInstance)
{
	if (!ModuleIconImage) return;

	if (UMaterialInstanceDynamic* IconMaterial = ModuleIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetScalarParameterValue(
			PARAM_ModuleIcon_SaturationAlpha,
			!ModuleInstance || ModuleInstance->IsActive() ? 1.f : 0.f);
	}
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
	if (CachedDefinition && IsValidSlot() && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
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
	CachedDefinition = ResolveDefinition();
	if (!CachedDefinition) return;

	UMASkillModuleDragDropOperation* DragOperation = NewObject<UMASkillModuleDragDropOperation>();
	if (!DragOperation) return;

	DragOperation->SourceOwner = SlotOwner;
	DragOperation->SourceSlots = SlotArray;
	DragOperation->SourceIndex = SlotIndex;
	DragOperation->Pivot = EDragPivot::CenterCenter;
	bIsHovered = false;
	RefreshHoverVisual();
	SetDraggedSourceVisual(true);

	const FMASkillDefinitionIconData IconData = CachedDefinition->ResolveIconData(UMAGameSettings::Get()->GetModuleQualityData());
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
	if (!DragOperation) return false;
	if (IsSelfDragOperation(DragOperation)) return true;

	return HandleDropFrom(
		DragOperation->SourceOwner.Get(),
		DragOperation->SourceSlots,
		DragOperation->SourceIndex);
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
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_HighlightAlpha, bHighlighted ? DropHighlightAlpha : 0.f);
		IconMaterial->SetScalarParameterValue(PARAM_ModuleIcon_IconScaleMultiplier, bHighlighted ? HighlightedIconScaleMultiplier : NormalIconScaleMultiplier);
	}
}

void UMASkillModuleSocketWidget::RefreshTooltip()
{
	if (!CachedDefinition || !TooltipWidgetClass)
	{
		SetToolTip(nullptr);
		return;
	}

	UMASkillTooltipWidget* TooltipWidget = CreateWidget<UMASkillTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
	if (!TooltipWidget)
	{
		SetToolTip(nullptr);
		return;
	}

	const UDataTable* WarningTextDataTable = ResolveWarningTextDataTable();
	const UMASkillModuleInstance* ModuleInstance = ResolveModuleInstance();
	TooltipWidget->SetSkillTooltip(
		CachedDefinition,
		ModuleInstance ? ModuleInstance->GetInactiveReasonTag() : FGameplayTag(),
		WarningTextDataTable);
	SetToolTip(TooltipWidget);
}

void UMASkillModuleSocketWidget::SetDraggedSourceVisual(bool bDragged)
{
	SetRenderOpacity(bDragged ? DraggedSourceRenderOpacity : 1.f);
}

bool UMASkillModuleSocketWidget::HandleDropFrom(
	UActorComponent* SourceOwner,
	const TArray<TObjectPtr<UMASkillModuleInstance>>* SourceSlots,
	int32 SourceIndex)
{
	if (!SourceOwner || !SourceSlots || SourceIndex == INDEX_NONE || !IsValidSlot()) return false;

	if (UMASkillManagerComponent* SourceSkillManager = Cast<UMASkillManagerComponent>(SourceOwner))
	{
		return SourceSkillManager->RequestMoveModuleSlot(
			SourceSlots,
			SourceIndex,
			SlotOwner.Get(),
			SlotArray,
			SlotIndex);
	}

	if (UMASkillModuleInventoryComponent* SourceInventory = Cast<UMASkillModuleInventoryComponent>(SourceOwner))
	{
		return SourceInventory->RequestMoveModuleSlot(
			SourceSlots,
			SourceIndex,
			SlotOwner.Get(),
			SlotArray,
			SlotIndex);
	}

	return false;
}

bool UMASkillModuleSocketWidget::IsSelfDragOperation(const UDragDropOperation* Operation) const
{
	const UMASkillModuleDragDropOperation* DragOperation = Cast<UMASkillModuleDragDropOperation>(Operation);
	return DragOperation
		&& DragOperation->SourceOwner == SlotOwner
		&& DragOperation->SourceSlots == SlotArray
		&& DragOperation->SourceIndex == SlotIndex;
}

