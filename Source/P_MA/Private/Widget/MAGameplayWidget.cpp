#include "Widget/MAGameplayWidget.h"

#include "Components/Button.h"
#include "Widget/MAAbilityListView.h"
#include "Widget/MAValueGauge.h"
#include "Widget/ShopWidget.h"
#include "Widget/SkillBookWidget.h"
#include "Widget/Skill/MASkillSlotWidget.h"
#include "Widget/Loop/LoopReadyWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GameFramework/PlayerController.h"

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
}

void UMAGameplayWidget::ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
    AbilityListView->ConfigureAbilities(Abilities);
}

void UMAGameplayWidget::ToggleShop()
{    
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return; 
    
    if (ActiveShopWidget && ActiveShopWidget->IsInViewport())
    {
        ActiveShopWidget->RemoveFromParent();
        ActiveShopWidget = nullptr;
        
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
    else
    {
        if (ShopWidgetClass)
        {
            ActiveShopWidget = CreateWidget<UShopWidget>(PC, ShopWidgetClass);
            if (ActiveShopWidget)
            {
                ActiveShopWidget->InitShop(ShopDataTables);
                ActiveShopWidget->AddToViewport(100);
                
                FInputModeGameAndUI InputMode;
                InputMode.SetWidgetToFocus(ActiveShopWidget->TakeWidget());
                InputMode.SetHideCursorDuringCapture(false);
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                
                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true;
            }
        }
    }
}

void UMAGameplayWidget::OnShopButtonClicked()
{
    ToggleShop();
}

void UMAGameplayWidget::ToggleSkillBook()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    if (ActiveSkillBookWidget && ActiveSkillBookWidget->IsInViewport())
    {
        // 닫을 때
        ActiveSkillBookWidget->RemoveFromParent();
        ActiveSkillBookWidget = nullptr;

        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
        /*
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true; */
    }
    else
    {
        // 열 때
        if (SkillBookWidgetClass)
        {
            ActiveSkillBookWidget = CreateWidget<USkillBookWidget>(PC, SkillBookWidgetClass);
            if (ActiveSkillBookWidget)
            {
                ActiveSkillBookWidget->AddToViewport(100);

                FInputModeGameAndUI InputMode;
                InputMode.SetHideCursorDuringCapture(false); 
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                InputMode.SetWidgetToFocus(ActiveSkillBookWidget->TakeWidget());

                PC->SetInputMode(InputMode);
                PC->bShowMouseCursor = true;
            }
        }
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
