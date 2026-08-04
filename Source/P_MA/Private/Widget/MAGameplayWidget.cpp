#include "Widget/MAGameplayWidget.h"

#include "Widget/MAValueGauge.h"
#include "Widget/MATemperatureGauge.h"
#include "Widget/Skill/MASkillSlotWidget.h"
#include "Widget/Skill/MASkillModuleInventoryWidget.h"
#include "Widget/Skill/MASkillPassiveModuleSlotsWidget.h"
#include "Widget/Loop/LoopReadyWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/Button.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "Inventory/MAInventoryComponent.h"

void UMAGameplayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ModuleInventoryToggleButton->OnClicked.AddUniqueDynamic(this, &UMAGameplayWidget::ToggleModuleInventory);
    
    UAbilitySystemComponent* OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
    if (OwnerAbilitySystemComponent && HealthBar)
    {
        HealthBar->Bind3Attributes(
            OwnerAbilitySystemComponent,
            UMAAttributeSet::GetHealthAttribute(),
            UMAAttributeSet::GetMaxHealthAttribute(),
            UMAAttributeSet::GetShieldAttribute());
    }

    if (OwnerAbilitySystemComponent && TemperatureBar)
    {
        TemperatureBar->BindTemperatureAttribute(OwnerAbilitySystemComponent);
    }

    APawn* OwningPawn = GetOwningPlayerPawn();
    UMASkillManagerComponent* SkillManager = OwningPawn
        ? OwningPawn->FindComponentByClass<UMASkillManagerComponent>()
        : nullptr;
    if (SkillManager)
    {
        SkillSlotWidget->InitializeSkillSlots(SkillManager);
        PassiveModuleSlotsWidget->InitializePassiveSlots(SkillManager);
    }

    if (SkillModuleInventoryWidget)
    {
        if (OwningPawn)
        {
            SkillModuleInventoryWidget->InitializeInventory(OwningPawn->FindComponentByClass<UMAInventoryComponent>());
        }
    }
}

void UMAGameplayWidget::ToggleModuleInventory()
{
	SkillModuleInventoryWidget->ToggleCollapsed();
}

void UMAGameplayWidget::ToggleSkillSlotsCollapsed()
{
	if (SkillSlotWidget)
	{
		SkillSlotWidget->ToggleRowsCollapsed();
		SkillModuleInventoryWidget->SetCollapsed(SkillSlotWidget->AreRowsCollapsed());
	}
}

void UMAGameplayWidget::SetLoopReadyVisible(bool bVisible)
{
	if (!LoopReadyWidget) return;

	const ESlateVisibility TargetVis = bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;
	LoopReadyWidget->SetVisibility(TargetVis);

	if (bVisible && !bLoopReadyInitialized)
	{
		RefreshLoopReady();
		bLoopReadyInitialized = true;
	}
}

void UMAGameplayWidget::RefreshLoopReady()
{
	if (!LoopReadyWidget) return;
    
	LoopReadyWidget->RefreshFromGameState();
}
