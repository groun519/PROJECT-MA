#include "Widget/Skill/MASkillPassiveModuleSlotsWidget.h"

#include "Components/PanelWidget.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "Widget/Skill/MASkillModuleSocketWidget.h"

void UMASkillPassiveModuleSlotsWidget::InitializePassiveSlots(UMASkillManagerComponent* InSkillManager)
{
	UnbindSkillManager();

	SkillManager = InSkillManager;
	RefreshSlots();

	if (SkillManager)
	{
		SkillSlotChangedHandle = SkillManager->OnSkillSlotChanged.AddUObject(
			this,
			&UMASkillPassiveModuleSlotsWidget::HandleSkillSlotChanged);
	}
}

void UMASkillPassiveModuleSlotsWidget::RefreshSlots()
{
	if (!SkillManager) return;

	const TArray<TObjectPtr<UMASkillModuleInstance>>* Slots = SkillManager->GetModuleSlotsForUI(
		FMASkillSystemStatics::GetPassiveSlotTag());

	EnsureSlotWidgets(Slots->Num());
	for (int32 SlotIndex = 0; SlotIndex < Slots->Num(); ++SlotIndex)
	{
		UMASkillModuleSocketWidget* SlotWidget = Cast<UMASkillModuleSocketWidget>(SlotContainer->GetChildAt(SlotIndex));
		if (!SlotWidget) continue;

		SlotWidget->InitializeSocket(SkillManager, Slots, SlotIndex);
	}
}

void UMASkillPassiveModuleSlotsWidget::NativeDestruct()
{
	UnbindSkillManager();
	Super::NativeDestruct();
}

void UMASkillPassiveModuleSlotsWidget::HandleSkillSlotChanged(FGameplayTag ChangedSlotTag)
{
	if (!FMASkillSystemStatics::IsPassiveSkillSlotTag(ChangedSlotTag)) return;

	RefreshSlots();
}

void UMASkillPassiveModuleSlotsWidget::UnbindSkillManager()
{
	if (SkillManager && SkillSlotChangedHandle.IsValid())
	{
		SkillManager->OnSkillSlotChanged.Remove(SkillSlotChangedHandle);
	}
	SkillSlotChangedHandle.Reset();
}

void UMASkillPassiveModuleSlotsWidget::EnsureSlotWidgets(int32 TargetCount)
{
	if (!SlotWidgetClass) return;

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
