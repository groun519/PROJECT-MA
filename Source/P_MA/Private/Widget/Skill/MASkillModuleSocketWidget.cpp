#include "Widget/Skill/MASkillModuleSocketWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"
#include "GAS/Skill/Addon/Cooldown/MASkillModuleCooldownAddon.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
#include "GAS/Skill/Module/MASkillModule.h"
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
	BindModuleState(ModuleInstance);
	CachedModule = ModuleInstance ? ModuleInstance->GetRootModule() : nullptr;
	ApplyModuleVisual(CachedModule);
	ApplyModuleStateVisual(ModuleInstance);
	RefreshCooldownVisual();
	RefreshStackText(ModuleInstance);
	RefreshTooltip();
}

UMASkillModuleInstance* UMASkillModuleSocketWidget::ResolveModuleInstance() const
{
	return IsValidSlot() ? (*SlotArray)[SlotIndex] : nullptr;
}

UMASkillModule* UMASkillModuleSocketWidget::ResolveModule() const
{
	UMASkillModuleInstance* ModuleInstance = ResolveModuleInstance();
	return ModuleInstance ? ModuleInstance->GetRootModule() : nullptr;
}

const UDataTable* UMASkillModuleSocketWidget::ResolveWarningTextDataTable() const
{
	return UMAGameSettings::Get()->GetWarningTextDataTable();
}

bool UMASkillModuleSocketWidget::IsValidSlot() const
{
	return SlotOwner.IsValid() && SlotArray && SlotArray->IsValidIndex(SlotIndex);
}

void UMASkillModuleSocketWidget::ApplyModuleVisual(const UMASkillModule* Module)
{
	if (!ModuleIconImage) return;

	const UMAModuleQualityData* ModuleQualityData = UMAGameSettings::Get()->GetModuleQualityData();
	const FMASkillIconData IconData = Module
		? Module->ResolveIconData(ModuleQualityData)
		: FMASkillIconData();
	const bool bHasIcon = IconData.Icon != nullptr;
	if (UMaterialInstanceDynamic* IconMaterial = ModuleIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_SubIconTexture, nullptr);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, Module ? Module->ResolveFrameColor(ModuleQualityData) : FLinearColor::White);
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

void UMASkillModuleSocketWidget::ApplyModuleStateVisual(const UMASkillModuleInstance* ModuleInstance)
{
	if (!ModuleIconImage) return;

	if (UMaterialInstanceDynamic* IconMaterial = ModuleIconImage->GetDynamicMaterial())
	{
		const bool bIsAvailable = !ModuleInstance
			|| (ModuleInstance->IsActive() && !ModuleInstance->IsCooldownActive());
		IconMaterial->SetScalarParameterValue(
			PARAM_ModuleIcon_SaturationAlpha,
			bIsAvailable ? 1.f : 0.f);
	}
}

void UMASkillModuleSocketWidget::RefreshCooldownDuration(
	const UMASkillModuleInstance* ModuleInstance)
{
	CooldownDurationSeconds = 0.f;
	if (!ModuleInstance) return;

	const UMASkillModuleCooldownAddon* CooldownAddon =
		MASkillModuleAddonStatics::FindAddon<UMASkillModuleCooldownAddon>(*ModuleInstance);
	if (CooldownAddon) CooldownDurationSeconds = CooldownAddon->GetDurationSeconds();
}

void UMASkillModuleSocketWidget::RefreshCooldownVisual()
{
	if (!CooldownOverlayImage) return;

	const UMASkillModuleInstance* ModuleInstance = BoundModuleInstance.Get();
	const float RemainingSeconds = ModuleInstance
		? ModuleInstance->GetCooldownRemainingSeconds()
		: 0.f;
	const bool bShowCooldown = CooldownDurationSeconds > 0.f && RemainingSeconds > 0.f;
	CooldownOverlayImage->SetVisibility(
		bShowCooldown ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (bShowCooldown)
	{
		if (UMaterialInstanceDynamic* CooldownMaterial = CooldownOverlayImage->GetDynamicMaterial())
		{
			const float CooldownAlpha = FMath::Clamp(
				1.f - RemainingSeconds / CooldownDurationSeconds,
				0.f,
				1.f);
			CooldownMaterial->SetScalarParameterValue(PARAM_ModuleIcon_CooldownAlpha, CooldownAlpha);
		}

		if (UWorld* World = GetWorld();
			World && !World->GetTimerManager().IsTimerActive(CooldownVisualTimerHandle))
		{
			World->GetTimerManager().SetTimer(
				CooldownVisualTimerHandle,
				this,
				&UMASkillModuleSocketWidget::RefreshCooldownVisual,
				CooldownVisualUpdateInterval,
				true);
		}
	}
	else
	{
		ClearCooldownVisualTimer();
	}
}

void UMASkillModuleSocketWidget::ClearCooldownVisualTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownVisualTimerHandle);
	}
}

void UMASkillModuleSocketWidget::RefreshStackText(const UMASkillModuleInstance* ModuleInstance)
{
	const UMASkillModule* Module = ModuleInstance ? ModuleInstance->GetRootModule() : nullptr;
	FText StackValueText;
	const bool bShowStack = Module
		&& Module->TryResolveSocketText(ModuleInstance->GetAddonRuntimeData(), StackValueText);
	StackText->SetVisibility(bShowStack ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (bShowStack)
	{
		StackText->SetText(StackValueText);
	}
}

void UMASkillModuleSocketWidget::BindModuleState(UMASkillModuleInstance* ModuleInstance)
{
	if (BoundModuleInstance == ModuleInstance)
	{
		RefreshCooldownDuration(ModuleInstance);
		return;
	}

	UnbindModuleState();
	BoundModuleInstance = ModuleInstance;
	RefreshCooldownDuration(ModuleInstance);
	if (ModuleInstance)
	{
		ModuleStateChangedHandle = ModuleInstance->OnStateChanged.AddUObject(
			this,
			&UMASkillModuleSocketWidget::HandleModuleStateChanged);
	}
}

void UMASkillModuleSocketWidget::UnbindModuleState()
{
	ClearCooldownVisualTimer();
	CooldownDurationSeconds = 0.f;

	if (UMASkillModuleInstance* ModuleInstance = BoundModuleInstance.Get())
	{
		ModuleInstance->OnStateChanged.Remove(ModuleStateChangedHandle);
	}

	ModuleStateChangedHandle.Reset();
	BoundModuleInstance.Reset();
}

void UMASkillModuleSocketWidget::HandleModuleStateChanged()
{
	UMASkillModuleInstance* ModuleInstance = BoundModuleInstance.Get();
	RefreshCooldownDuration(ModuleInstance);
	ApplyModuleStateVisual(ModuleInstance);
	RefreshCooldownVisual();
	RefreshStackText(ModuleInstance);
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
	if (CachedModule && IsValidSlot() && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
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
	CachedModule = ResolveModule();
	if (!CachedModule) return;

	UMASkillModuleDragDropOperation* DragOperation = NewObject<UMASkillModuleDragDropOperation>();
	if (!DragOperation) return;

	DragOperation->SourceOwner = SlotOwner;
	DragOperation->SourceSlots = SlotArray;
	DragOperation->SourceIndex = SlotIndex;
	DragOperation->Pivot = EDragPivot::CenterCenter;
	bIsHovered = false;
	RefreshHoverVisual();
	SetDraggedSourceVisual(true);

	const FMASkillIconData IconData = CachedModule->ResolveIconData(UMAGameSettings::Get()->GetModuleQualityData());
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
	UnbindModuleState();
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
	if (!CachedModule || !TooltipWidgetClass)
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
	if (ModuleInstance)
	{
		TooltipWidget->SetModuleTooltip(*ModuleInstance, WarningTextDataTable);
	}
	else
	{
		TooltipWidget->SetSkillTooltip(CachedModule, FGameplayTag(), WarningTextDataTable);
	}
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


