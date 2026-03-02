// Fill out your copyright notice in the Description page of Project Settings.


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
#include "Widget/Loop/LoopReadyWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Level/Platform/Core.h"
#include "GameFramework/PlayerController.h" 
#include "Engine/World.h"

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

    if (CoreHealthBar)
    {
        if (!TryBindCoreHealthFromWorld())
        {
            if (UWorld* World = GetWorld())
            {
                CoreSpawnedHandle = World->AddOnActorSpawnedHandler(
                    FOnActorSpawned::FDelegate::CreateUObject(this, &UMAGameplayWidget::HandleActorSpawned)
                );
            }
        }
    }

}

void UMAGameplayWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        if (CoreSpawnedHandle.IsValid())
        {
            World->RemoveOnActorSpawnedHandler(CoreSpawnedHandle);
            CoreSpawnedHandle.Reset();
        }
    }
    Super::NativeDestruct();
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
        
        FInputModeGameOnly InputMode;
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

        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true; 
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
	if (!LoopReadyWidget)
	{
		return;
	}

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
	if (!LoopReadyWidget)
	{
		return;
	}
	LoopReadyWidget->RefreshFromGameState();
}

bool UMAGameplayWidget::TryBindCoreHealthFromWorld()
{
    if (bCoreHealthBound || !CoreHealthBar)
    {
        return bCoreHealthBound;
    }

    ACore* CoreActor = Cast<ACore>(UGameplayStatics::GetActorOfClass(GetWorld(), ACore::StaticClass()));
    if (CoreActor)
    {
        TryBindCoreHealthFromActor(CoreActor);
    }

    return bCoreHealthBound;
}

void UMAGameplayWidget::TryBindCoreHealthFromActor(ACore* CoreActor)
{
    if (bCoreHealthBound || !CoreHealthBar || !CoreActor)
    {
        return;
    }

    UAbilitySystemComponent* CoreASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CoreActor);
    if (CoreASC)
    {
        CoreHealthBar->SetAndBoundToGameplayAttribute(
            CoreASC,
            UMAAttributeSet::GetHealthAttribute(),
            UMAAttributeSet::GetMaxHealthAttribute());
        bCoreHealthBound = true;

        if (UWorld* World = GetWorld())
        {
            if (CoreSpawnedHandle.IsValid())
            {
                World->RemoveOnActorSpawnedHandler(CoreSpawnedHandle);
                CoreSpawnedHandle.Reset();
            }
        }
    }
}

void UMAGameplayWidget::HandleActorSpawned(AActor* SpawnedActor)
{
    if (bCoreHealthBound || !SpawnedActor)
    {
        return;
    }

    if (ACore* CoreActor = Cast<ACore>(SpawnedActor))
    {
        TryBindCoreHealthFromActor(CoreActor);
    }
}
