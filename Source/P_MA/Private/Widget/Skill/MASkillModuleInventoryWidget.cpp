#include "Widget/Skill/MASkillModuleInventoryWidget.h"

#include "Components/PanelWidget.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
#include "Widget/Skill/MASkillModuleSocketWidget.h"

void UMASkillModuleInventoryWidget::InitializeInventory(UMASkillModuleInventoryComponent* InInventory)
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

	const TArray<TObjectPtr<UMASkillDefinition>>* Slots = Inventory->GetModuleSlotsForUI();
	if (!Slots) return;

	EnsureSlotWidgets(Slots->Num());
	for (int32 SlotIndex = 0; SlotIndex < Slots->Num(); ++SlotIndex)
	{
		UMASkillModuleSocketWidget* SlotWidget = Cast<UMASkillModuleSocketWidget>(SlotContainer->GetChildAt(SlotIndex));
		if (!SlotWidget) continue;

		SlotWidget->InitializeSocket(Inventory, Slots, SlotIndex);
	}
}

void UMASkillModuleInventoryWidget::NativeDestruct()
{
	UnbindInventory();
	Super::NativeDestruct();
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
