#include "Widget/Skill/MASkillSlotWidget.h"

#include "Components/PanelWidget.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GameFramework/PlayerController.h"
#include "Widget/Skill/MASkillSlotRowWidget.h"

void UMASkillSlotWidget::InitializeSkillSlots(UMASkillManagerComponent* InSkillManager)
{
	SkillManager = InSkillManager;
	RebuildSlotRows();
}

void UMASkillSlotWidget::ToggleRowsCollapsed()
{
	SetRowsCollapsed(!bRowsCollapsed);
}

void UMASkillSlotWidget::SetRowsCollapsed(bool bCollapsed)
{
	bRowsCollapsed = bCollapsed;

	if (!SlotRowsBox) return;

	for (UWidget* ChildWidget : SlotRowsBox->GetAllChildren())
	{
		if (UMASkillSlotRowWidget* RowWidget = Cast<UMASkillSlotRowWidget>(ChildWidget))
		{
			RowWidget->SetCollapsed(bRowsCollapsed);
		}
	}
}

void UMASkillSlotWidget::RebuildSlotRows()
{
	if (!SlotRowsBox) return;

	SlotRowsBox->ClearChildren();
	if (!SkillManager || !SlotRowWidgetClass) return;

	const TArray<FGameplayTag> SlotTags = SkillManager->GetSkillSlotTags();
	APlayerController* OwningPlayer = GetOwningPlayer();
	for (const FGameplayTag& SlotTag : SlotTags)
	{
		UMASkillSlotRowWidget* RowWidget = OwningPlayer
			? CreateWidget<UMASkillSlotRowWidget>(OwningPlayer, SlotRowWidgetClass)
			: CreateWidget<UMASkillSlotRowWidget>(this, SlotRowWidgetClass);
		if (!RowWidget) continue;

		SlotRowsBox->AddChild(RowWidget);
		RowWidget->InitializeSlot(SkillManager, SlotTag);
		RowWidget->SetCollapsed(bRowsCollapsed);
	}
}
