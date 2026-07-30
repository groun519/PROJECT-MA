#include "Widget/Skill/MASkillModuleSocketWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"
#include "GAS/Skill/Addon/Cooldown/MASkillModuleCooldownAddon.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "Inventory/MAInventoryComponent.h"
#include "Item/Data/MAItemData.h"
#include "Item/MAItemType.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Skill/MASkillModuleDragDropOperation.h"
#include "Widget/Skill/MASkillModuleDragVisualWidget.h"
#include "Widget/Skill/MASkillTooltipWidget.h"

/** Initialization **/
void UMASkillModuleSocketWidget::InitializeInventorySlot(
	UMAInventoryComponent* InInventory,
	const int32 InSlotIndex)
{
	Inventory = InInventory;
	SkillManager.Reset();
	SkillSlotTag = FGameplayTag();
	SlotIndex = InSlotIndex;

	Refresh();
}

void UMASkillModuleSocketWidget::InitializeSkillSlot(
	UMASkillManagerComponent* InSkillManager,
	const FGameplayTag InSkillSlotTag,
	const int32 InSlotIndex)
{
	Inventory.Reset();
	SkillManager = InSkillManager;
	SkillSlotTag = InSkillSlotTag;
	SlotIndex = InSlotIndex;

	Refresh();
}

/** Slot Content **/
void UMASkillModuleSocketWidget::Refresh()
{
	const FMAInventoryEntry* InventoryEntry = ResolveInventoryEntry();
	DisplayedEntryId = InventoryEntry ? InventoryEntry->EntryId : INDEX_NONE;

	if (!ModuleIconImage) return;

	SetDraggedSourceVisual(false);
	bIsHovered = false;
	bIsDropTargetHighlighted = false;
	RefreshHoverVisual();

	UMASkillModuleInstance* ModuleInstance = ResolveModuleInstance();
	BindModuleState(ModuleInstance);
	CachedModule = ModuleInstance ? ModuleInstance->GetRootModule() : nullptr;

	if (InventoryEntry && InventoryEntry->IsItem())
	{
		ApplyItemVisual(InventoryEntry->ItemStack);
		SetStackText(InventoryEntry->ItemStack.Count > 1
			? FText::AsNumber(InventoryEntry->ItemStack.Count)
			: FText());
	}
	else
	{
		ApplyModuleVisual(CachedModule);
		RefreshModuleStackText(ModuleInstance);
	}

	ApplyModuleStateVisual(ModuleInstance);
	RefreshCooldownVisual();
	RefreshTooltip(InventoryEntry);
}

const FMAInventoryEntry* UMASkillModuleSocketWidget::ResolveInventoryEntry() const
{
	return Inventory.IsValid() ? Inventory->GetEntryAt(SlotIndex) : nullptr;
}

UMASkillModuleInstance* UMASkillModuleSocketWidget::ResolveModuleInstance() const
{
	if (Inventory.IsValid()) return Inventory->GetModuleAt(SlotIndex);
	if (SkillManager.IsValid()) return SkillManager->GetModuleInstanceAt(SkillSlotTag, SlotIndex);
	return nullptr;
}

const FMAItemDataRow* UMASkillModuleSocketWidget::ResolveItemData(
	const FMAItemStack& ItemStack) const
{
	const FMAItemId& ItemId = ItemStack.ItemId;
	const UMAItemType* ItemType = ItemId.GetItemType();
	return ItemType
		? ItemType->FindItemData(ItemId.RowName)
		: nullptr;
}

/** Visuals **/
void UMASkillModuleSocketWidget::ApplyModuleVisual(const UMASkillModule* Module)
{
	const UMAModuleQualityData* ModuleQualityData = UMAGameSettings::Get()->GetModuleQualityData();
	const FMASkillIconData IconData = Module
		? Module->ResolveIconData(ModuleQualityData)
		: FMASkillIconData();
	ApplyIconVisual(
		IconData,
		Module ? Module->ResolveFrameColor(ModuleQualityData) : FLinearColor::White);
}

void UMASkillModuleSocketWidget::ApplyItemVisual(const FMAItemStack& ItemStack)
{
	FMASkillIconData IconData;
	if (const FMAItemDataRow* ItemData = ResolveItemData(ItemStack))
	{
		IconData.Icon = ItemData->Icon.LoadSynchronous();
	}
	ApplyIconVisual(IconData, FLinearColor::White);
}

void UMASkillModuleSocketWidget::ApplyIconVisual(
	const FMASkillIconData& IconData,
	const FLinearColor& FrameColor)
{
	if (!ModuleIconImage) return;

	const bool bHasIcon = IconData.Icon != nullptr;
	if (UMaterialInstanceDynamic* IconMaterial = ModuleIconImage->GetDynamicMaterial())
	{
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_IconTexture, IconData.Icon);
		IconMaterial->SetTextureParameterValue(PARAM_ModuleIcon_SubIconTexture, nullptr);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_IconColor, IconData.IconColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_InnerColor, IconData.InnerColor);
		IconMaterial->SetVectorParameterValue(PARAM_ModuleIcon_FrameColor, FrameColor);
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

/** Module State **/
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

void UMASkillModuleSocketWidget::RefreshModuleStackText(
	const UMASkillModuleInstance* ModuleInstance)
{
	const UMASkillModule* Module = ModuleInstance ? ModuleInstance->GetRootModule() : nullptr;
	FText StackValueText;
	if (Module)
	{
		Module->TryResolveSocketText(ModuleInstance->GetAddonRuntimeData(), StackValueText);
	}
	SetStackText(StackValueText);
}

void UMASkillModuleSocketWidget::SetStackText(const FText& Text)
{
	const bool bShowText = !Text.IsEmpty();
	StackText->SetVisibility(
		bShowText ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	if (bShowText) StackText->SetText(Text);
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
	RefreshModuleStackText(ModuleInstance);
}

/** Interaction **/
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
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && Inventory.IsValid())
	{
		Inventory->UseEntry(DisplayedEntryId);
		return FReply::Handled();
	}

	const FMAInventoryEntry* InventoryEntry = ResolveInventoryEntry();
	const bool bHasDraggableEntry = CachedModule
		|| (InventoryEntry && InventoryEntry->IsItem());
	if (bHasDraggableEntry && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
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
	UMASkillModuleDragDropOperation* DragOperation = NewObject<UMASkillModuleDragDropOperation>();
	if (!DragOperation) return;

	UTexture2D* DragIcon = nullptr;
	FLinearColor DragIconColor = FLinearColor::White;
	if (Inventory.IsValid())
	{
		const FMAInventoryEntry* Entry = ResolveInventoryEntry();
		if (!Entry || !DragOperation->SetSource(Inventory.Get(), *Entry)) return;

		if (Entry->IsModule())
		{
			CachedModule = Entry->ModuleInstance->GetRootModule();
			const FMASkillIconData IconData = CachedModule
				? CachedModule->ResolveIconData(UMAGameSettings::Get()->GetModuleQualityData())
				: FMASkillIconData();
			DragIcon = IconData.Icon;
			DragIconColor = IconData.IconColor;
		}
		else if (const FMAItemDataRow* ItemData = ResolveItemData(Entry->ItemStack))
		{
			DragIcon = ItemData->Icon.LoadSynchronous();
		}
	}
	else if (SkillManager.IsValid())
	{
		UMASkillModuleInstance* ModuleInstance = ResolveModuleInstance();
		CachedModule = ModuleInstance ? ModuleInstance->GetRootModule() : nullptr;
		if (!CachedModule) return;
		if (!DragOperation->SetSource(SkillManager.Get(), SkillSlotTag, SlotIndex)) return;

		const FMASkillIconData IconData = CachedModule->ResolveIconData(
			UMAGameSettings::Get()->GetModuleQualityData());
		DragIcon = IconData.Icon;
		DragIconColor = IconData.IconColor;
	}
	else
	{
		return;
	}
	DragOperation->Pivot = EDragPivot::CenterCenter;
	DragOperation->OnDrop.AddUniqueDynamic(
		this,
		&UMASkillModuleSocketWidget::HandleDragCompleted);
	bIsHovered = false;
	RefreshHoverVisual();
	SetDraggedSourceVisual(true);

	if (DragVisualWidgetClass && DragIcon)
	{
		UMASkillModuleDragVisualWidget* DragVisual = CreateWidget<UMASkillModuleDragVisualWidget>(this, DragVisualWidgetClass);
		if (DragVisual)
		{
			DragVisual->SetIcon(DragIcon, DragIconColor);
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

	const UMASkillModuleDragDropOperation* DragOperation =
		Cast<UMASkillModuleDragDropOperation>(InOperation);
	bool bCanDrop = false;
	if (DragOperation)
	{
		if (Inventory.IsValid())
		{
			bCanDrop = DragOperation->CanDropOn(Inventory.Get(), SlotIndex);
		}
		else if (SkillManager.IsValid())
		{
			bCanDrop = DragOperation->CanDropOn(SkillManager.Get(), SkillSlotTag, SlotIndex);
		}
	}
	if (bCanDrop)
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

	UMASkillModuleDragDropOperation* DragOperation = Cast<UMASkillModuleDragDropOperation>(InOperation);
	if (!DragOperation) return false;

	if (Inventory.IsValid())
	{
		return DragOperation->TryDropOn(Inventory.Get(), SlotIndex);
	}
	if (SkillManager.IsValid())
	{
		return DragOperation->TryDropOn(SkillManager.Get(), SkillSlotTag, SlotIndex);
	}
	return false;
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

void UMASkillModuleSocketWidget::RefreshTooltip(const FMAInventoryEntry* InventoryEntry)
{
	SetToolTip(nullptr);
	SetToolTipText(FText());

	if (InventoryEntry && InventoryEntry->IsItem())
	{
		if (const FMAItemDataRow* ItemData = ResolveItemData(InventoryEntry->ItemStack))
		{
			SetToolTipText(ItemData->DisplayName);
		}
		return;
	}

	if (!CachedModule || !TooltipWidgetClass)
	{
		return;
	}

	UMASkillTooltipWidget* TooltipWidget = CreateWidget<UMASkillTooltipWidget>(GetOwningPlayer(), TooltipWidgetClass);
	if (!TooltipWidget)
	{
		SetToolTip(nullptr);
		return;
	}

	const UDataTable* WarningTextDataTable = UMAGameSettings::Get()->GetWarningTextDataTable();
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

void UMASkillModuleSocketWidget::HandleDragCompleted(UDragDropOperation*)
{
	SetDraggedSourceVisual(false);
}
