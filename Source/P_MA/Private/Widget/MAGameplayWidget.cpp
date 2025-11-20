#include "Widget/MAGameplayWidget.h"
#include "Widget/MASkillSlotWidget.h"
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
        // OnClicked 이벤트에 C++ 함수를 연결(바인딩)합니다.
        // 이 한 줄이 블루프린트 이벤트 그래프에서 노드를 연결하는 것과 똑같은 역할을 합니다.
        ShopButton->OnClicked.AddDynamic(this, &UMAGameplayWidget::OnShopButtonClicked);
    }

    // AbilitySystemComponent을 통한 체력바 설정
    UAbilitySystemComponent* OwnerAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
    if (OwnerAbilitySystemComponent && HealthBar)
    {
        HealthBar->SetAndBoundToGameplayAttribute(OwnerAbilitySystemComponent, UMAAttributeSet::GetHealthAttribute(), UMAAttributeSet::GetMaxHealthAttribute());
    }

    // 스킬 슬롯과 패시브 슬롯 생성
    // CreateSkillSlots(2);  // 예시: 2개의 스킬 슬롯
    // CreatePassiveSlots(6);  // 예시: 6개의 패시브 슬롯
}

// void UMAGameplayWidget::CreateSkillSlots(int32 NumSlots)
// {
//     if (!SkillSlotWidgetClass || !HorizontalBox_SkillSlots) return;
//
//     for (int32 i = 0; i < NumSlots; ++i)
//     {
//         UMASkillSlotWidget* NewSlot = CreateWidget<UMASkillSlotWidget>(GetWorld(), SkillSlotWidgetClass);
//
//         // 핫키 이름을 설정
//         FString KeyName = (i == 0) ? TEXT("Attack") : TEXT("Skill");
//         NewSlot->SetHotkeyName(KeyName);
//
//         // `HorizontalBox`에 추가
//         UHorizontalBoxSlot* NewSlotSlot = HorizontalBox_SkillSlots->AddChildToHorizontalBox(NewSlot);
//         if (NewSlotSlot)
//         {
//             NewSlotSlot->SetPadding(FMargin(5.0f, 0.0f));  // 각 슬롯 간의 여백
//             NewSlotSlot->SetHorizontalAlignment(HAlign_Left);  // 수평 정렬
//         }
//     }
// }

// void UMAGameplayWidget::CreatePassiveSlots(int32 NumSlots)
// {
//     if (!PassiveSlotWidgetClass || !HorizontalBox_PassiveSlots) return;
//
//     const int32 MaxSlots = 6;
//
//     for (int32 i = 0; i < MaxSlots; ++i)
//     {
//         UMAPassiveSlotWidget* NewSlot = CreateWidget<UMAPassiveSlotWidget>(GetWorld(), PassiveSlotWidgetClass);
//         UHorizontalBoxSlot* NewSlotSlot = HorizontalBox_PassiveSlots->AddChildToHorizontalBox(NewSlot);
//
//         if (NewSlotSlot)
//         {
//             NewSlotSlot->SetPadding(FMargin(3.0f, 0.0f));  // 각 슬롯 간의 여백
//             NewSlotSlot->SetHorizontalAlignment(HAlign_Fill);  // 수평 정렬
//             NewSlotSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));  // 슬롯 크기 설정
//         }
//
//         // 슬롯을 표시하거나 숨깁니다. (예시로 3개만 보이도록 설정)
//         NewSlot->SetVisibility(i < 3 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
//     }
// }

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
        // 켜져 있으면 -> 끔
        SkillBookWidget->SetVisibility(ESlateVisibility::Hidden);
        SetOwinigPawnInputEnabled(true); // 게임 조작 활성화
    }
    else
    {
        // 꺼져 있으면 -> 켬
        SkillBookWidget->SetVisibility(ESlateVisibility::Visible);
        SetOwinigPawnInputEnabled(false); // 게임 조작 비활성화 (UI 모드)
    }
}