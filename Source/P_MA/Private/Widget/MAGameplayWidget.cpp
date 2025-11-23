#include "Widget/MAGameplayWidget.h"
#include "Widget/MAPassiveSlotWidget.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Button.h"
#include "Widget/MAAbilityListView.h"
#include "Widget/MAValueGauge.h"
#include "Widget/ShopWidget.h"
#include "Widget/SkillBookWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAAttributeSet.h"

void UMAGameplayWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ShopButton)
    {
        ShopButton->OnClicked.AddDynamic(this, &UMAGameplayWidget::OnShopButtonClicked);
    }
    
    UAbilitySystemComponent* OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
    if (OwnerAbilitySystemComponent && HealthBar)
    {
        HealthBar->SetAndBoundToGameplayAttribute(OwnerAbilitySystemComponent, UMAAttributeSet::GetHealthAttribute(), UMAAttributeSet::GetMaxHealthAttribute());
    }
    
}

void UMAGameplayWidget::ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
    AbilityListView->ConfigureAbilities(Abilities);
}


void UMAGameplayWidget::ToggleShop()
{
    if (ShopWidget->GetVisibility() == ESlateVisibility::HitTestInvisible)
    {
        ShopWidget->SetVisibility(ESlateVisibility::Visible);
        PlayShopPopupAnimation(true);
        SetOwinigPawnInputEnabled(false);
    }
    else
    {
        ShopWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
        PlayShopPopupAnimation(false);
        SetOwinigPawnInputEnabled(true);
    }
}

void UMAGameplayWidget::PlayShopPopupAnimation(bool bPlayForward)
{
    if (bPlayForward)
    {
        PlayAnimationForward(ShopPopupAnimation);
    }
    else
    {
        PlayAnimationReverse(ShopPopupAnimation);
    }
}
void UMAGameplayWidget::SetOwinigPawnInputEnabled(bool bPawnInputEnabled)
{
    if (bPawnInputEnabled)
    {
        GetOwningPlayerPawn()->EnableInput(GetOwningPlayer());
    }
    else
    {
        GetOwningPlayerPawn()->DisableInput(GetOwningPlayer());
    }
}

void UMAGameplayWidget::OnShopButtonClicked()
{
    ToggleShop();
}

void UMAGameplayWidget::ToggleSkillBook()
{
    if (!SkillBookWidget) return;

    if (SkillBookWidget->GetVisibility() == ESlateVisibility::Visible)
    {
        SkillBookWidget->SetVisibility(ESlateVisibility::Hidden);
        SetOwinigPawnInputEnabled(true);
    }
    else
    {
        SkillBookWidget->SetVisibility(ESlateVisibility::Visible);
        SetOwinigPawnInputEnabled(false); 
    }
}