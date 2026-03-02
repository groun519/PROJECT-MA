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
#include "GameFramework/PlayerController.h" // [필수] InputMode 설정을 위해 추가

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

void UMAGameplayWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UMAGameplayWidget::ConfigureAbilities(const TMap<EMAAbilityInputID, TSubclassOf<class UGameplayAbility>>& Abilities)
{
    AbilityListView->ConfigureAbilities(Abilities);
}

void UMAGameplayWidget::ToggleShop()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return; // 💡 안전을 위한 널 체크 추가
    
    if (ActiveShopWidget && ActiveShopWidget->IsInViewport())
    {
        ActiveShopWidget->RemoveFromParent();
        ActiveShopWidget = nullptr;
        
        // 💡 닫을 때 게임 전용 모드로 복구
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
                
                // ✨ 상점도 스킬북처럼 깜빡임 방지 옵션 적용!
                FInputModeGameAndUI InputMode;
                InputMode.SetWidgetToFocus(ActiveShopWidget->TakeWidget());
                InputMode.SetHideCursorDuringCapture(false); // 👈 클릭 시 커서 사라짐 방지
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

        // 💡 닫을 때 마우스 상태가 꼬이지 않게 게임 전용 모드로 확실히 돌려줍니다.
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true; // 범님 설정 유지
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

                // ✨ 마우스 깜빡임 해결의 핵심 세팅!
                FInputModeGameAndUI InputMode;
                // 1. 클릭해도 마우스를 숨기지 않도록 설정 (깜빡임 방지)
                InputMode.SetHideCursorDuringCapture(false); 
                // 2. 마우스가 화면 밖으로 나가는 걸 막지 않음 (자유로운 마우스)
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
                // 3. 새로 만든 위젯에 포커스를 줘서 입력을 가로채게 함
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
