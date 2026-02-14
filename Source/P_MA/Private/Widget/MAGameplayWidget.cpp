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
#include "GameFramework/PlayerController.h" // [필수] InputMode 설정을 위해 추가
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
    // 기존 HitTestInvisible 로직 유지하되 InputMode 변경 적용
    if (ShopWidget->GetVisibility() == ESlateVisibility::HitTestInvisible)
    {
        // 상점 열기
        ShopWidget->SetVisibility(ESlateVisibility::Visible);
        PlayShopPopupAnimation(true);
    }
    else
    {
        // 상점 닫기
        ShopWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
        PlayShopPopupAnimation(false);
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

void UMAGameplayWidget::OnShopButtonClicked()
{
    ToggleShop();
}

void UMAGameplayWidget::ToggleSkillBook()
{
    if (!SkillBookWidget) return;

    if (SkillBookWidget->GetVisibility() == ESlateVisibility::Visible)
    {
        // 스킬북 닫기
        if(SkillBookPopupAnimation)
        {
            PlayAnimationReverse(SkillBookPopupAnimation);
        }
        
        SkillBookWidget->SetVisibility(ESlateVisibility::Hidden);
    }
    else
    {
        // 스킬북 열기
        SkillBookWidget->SetVisibility(ESlateVisibility::Visible);
        
        if(SkillBookPopupAnimation)
        {
            PlayAnimationForward(SkillBookPopupAnimation);
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
