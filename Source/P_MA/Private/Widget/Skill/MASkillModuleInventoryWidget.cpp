#include "Widget/Skill/MASkillModuleInventoryWidget.h"

#include "Components/PanelWidget.h"
#include "Inventory/MAInventoryComponent.h"
#include "Widget/Skill/MASkillModuleSocketWidget.h"

void UMASkillModuleInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bIsCollapsed = GetVisibility() == ESlateVisibility::Collapsed;
}

void UMASkillModuleInventoryWidget::InitializeInventory(UMAInventoryComponent* InInventory)
{
	UnbindInventory();

	Inventory = InInventory;
	RefreshSlots();

	if (Inventory)
	{
		InventoryChangedHandle = Inventory->OnInventoryChanged.AddUObject(this, &UMASkillModuleInventoryWidget::RefreshSlots);
	}
}

void UMASkillModuleInventoryWidget::RefreshSlots()
{
	if (!SlotContainer) return;
	if (!Inventory) return;

	const int32 SlotCount = Inventory->GetEntries().Num();
	EnsureSlotWidgets(SlotCount);
	for (int32 SlotIndex = 0; SlotIndex < SlotCount; ++SlotIndex)
	{
		UMASkillModuleSocketWidget* SlotWidget = Cast<UMASkillModuleSocketWidget>(SlotContainer->GetChildAt(SlotIndex));
		if (!SlotWidget) continue;

		SlotWidget->InitializeInventorySlot(Inventory, SlotIndex);
	}
}

void UMASkillModuleInventoryWidget::ToggleCollapsed()
{
	SetCollapsed(!bIsCollapsed);
}

void UMASkillModuleInventoryWidget::SetCollapsed(bool bCollapsed)
{
	if (bIsCollapsed == bCollapsed) return;

	bIsCollapsed = bCollapsed;
	if (!InventoryCollapse)
	{
		SetVisibility(bIsCollapsed ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		return;
	}

	StopAnimation(InventoryCollapse);
	SetVisibility(ESlateVisibility::Visible);
	if (bIsCollapsed)
	{
		PlayAnimationForward(InventoryCollapse);
	}
	else
	{
		PlayAnimationReverse(InventoryCollapse);
	}
}

void UMASkillModuleInventoryWidget::NativeDestruct()
{
	UnbindInventory();
	Super::NativeDestruct();
}

void UMASkillModuleInventoryWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	if (Animation == InventoryCollapse && bIsCollapsed)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UMASkillModuleInventoryWidget::UnbindInventory()
{
	if (Inventory && InventoryChangedHandle.IsValid())
	{
		Inventory->OnInventoryChanged.Remove(InventoryChangedHandle);
	}
	InventoryChangedHandle.Reset();
}

void UMASkillModuleInventoryWidget::EnsureSlotWidgets(int32 TargetCount)
{
	if (!SlotContainer || !SlotWidgetClass) return;

	while (SlotContainer->GetChildrenCount() < TargetCount)
	{
		UMASkillModuleSocketWidget* SlotWidget = CreateWidget<UMASkillModuleSocketWidget>(this, SlotWidgetClass);
		if (!SlotWidget) return;

		SlotContainer->AddChild(SlotWidget);
	}
	while (SlotContainer->GetChildrenCount() > TargetCount)
	{
		SlotContainer->RemoveChildAt(SlotContainer->GetChildrenCount() - 1);
	}
}
