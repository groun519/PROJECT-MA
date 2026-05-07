#include "Widget/Skill/MASkillSlotWidget.h"

#include "Components/PanelWidget.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "Widget/Skill/MASkillSlotRowWidget.h"

void UMASkillSlotWidget::InitializeSkillSlots(UMASkillManagerComponent* InSkillManager)
{
	SkillManager = InSkillManager;
	RebuildSlotRows();
}

void UMASkillSlotWidget::RebuildSlotRows()
{
	if (!SlotRowsBox) return;

	SlotRowsBox->ClearChildren();
	if (!SkillManager || !SlotRowWidgetClass) return;

	const TArray<EMAAbilityInputID> InputIDs = SkillManager->GetSkillSlotInputIDs();
	for (const EMAAbilityInputID InputID : InputIDs)
	{
		UMASkillSlotRowWidget* RowWidget = CreateWidget<UMASkillSlotRowWidget>(this, SlotRowWidgetClass);
		if (!RowWidget) continue;

		RowWidget->InitializeSlot(SkillManager, InputID);
		SlotRowsBox->AddChild(RowWidget);
	}
}
