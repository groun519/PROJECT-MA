#include "Widget/MAGameplayWidget.h"

#include "Widget/MAValueGauge.h"
#include "Widget/Skill/MASkillSlotWidget.h"
#include "Widget/Skill/MASkillModuleInventoryWidget.h"
#include "Widget/Loop/LoopReadyWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"

void UMAGameplayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    UAbilitySystemComponent* OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
    if (OwnerAbilitySystemComponent && HealthBar)
    {
        HealthBar->Bind3Attributes(
            OwnerAbilitySystemComponent,
            UMAAttributeSet::GetHealthAttribute(),
            UMAAttributeSet::GetMaxHealthAttribute(),
            UMAAttributeSet::GetShieldAttribute());
    }

    if (SkillSlotWidget)
    {
        if (APawn* OwningPawn = GetOwningPlayerPawn())
        {
            if (UMASkillManagerComponent* SkillManager = OwningPawn->FindComponentByClass<UMASkillManagerComponent>())
            {
                SkillSlotWidget->InitializeSkillSlots(SkillManager);
            }
        }
    }

    if (SkillModuleInventoryWidget)
    {
        if (APawn* OwningPawn = GetOwningPlayerPawn())
        {
            SkillModuleInventoryWidget->InitializeInventory(OwningPawn->FindComponentByClass<UMASkillModuleInventoryComponent>());
        }
    }
}

void UMAGameplayWidget::ToggleSkillSlotsCollapsed()
{
	if (SkillSlotWidget)
	{
		SkillSlotWidget->ToggleRowsCollapsed();
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
