#include "Widget/MAHUD.h"
#include "Widget/MASkillSlotWidget.h"
#include "Widget/HealthBarWidget.h"
#include "Widget/MAPassiveSlotWidget.h"

#include "Blueprint/UserWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

void UMAHUD::NativeConstruct()
{
    Super::NativeConstruct();

    if (HealthBarWidgetClass)
    {
        HealthBarWidget = CreateWidget<UHealthBarWidget>(GetWorld(), HealthBarWidgetClass);
        if (HealthBarWidget)
        {
            HealthBarWidget->AddToViewport();
            HealthBarWidget->SetHealthPercent(1.0f);
        }
    }

    CreateSkillSlots(2);
    CreatePassiveSlots(6);
}

void UMAHUD::UpdateHealth(float CurrentHealth, float MaxHealth)
{
    if (HealthBarWidget)
    {
        HealthBarWidget->SetHealthPercent(CurrentHealth / MaxHealth);
    }
}

void UMAHUD::CreateSkillSlots(int32 NumSlots)
{
    if (!SkillSlotWidgetClass || !HorizontalBox_SkillSlots) return;

    for (int32 i = 0; i < NumSlots; ++i)
    {
        UMASkillSlotWidget* NewSlot = CreateWidget<UMASkillSlotWidget>(GetWorld(), SkillSlotWidgetClass);

        FString KeyName = (i == 0) ? TEXT("Attack") : TEXT("Skill");
        NewSlot->SetHotkeyName(KeyName);

        UHorizontalBoxSlot* NewSlotSlot = HorizontalBox_SkillSlots->AddChildToHorizontalBox(NewSlot);
        if (NewSlotSlot)
        {
            NewSlotSlot->SetPadding(FMargin(5.0f, 0.0f));
            NewSlotSlot->SetHorizontalAlignment(HAlign_Left);
        }
    }
}

void UMAHUD::CreatePassiveSlots(int32 NumSlots)
{
    if (!PassiveSlotWidgetClass || !HorizontalBox_PassiveSlots) return;

    const int32 MaxSlots = 6;

    // TArray<UPassiveData*> PassiveArray = GetPassiveArrayFromCharacter(); 
    // const int32 VisibleCount = PassiveArray.Num();

    const int32 VisibleCount = 3; // 테스트용 임시 숫자 (나중에 없앨 것)

    for (int32 i = 0; i < MaxSlots; ++i)
    {
        UMAPassiveSlotWidget* NewSlot = CreateWidget<UMAPassiveSlotWidget>(GetWorld(), PassiveSlotWidgetClass);
        UHorizontalBoxSlot* NewSlotSlot = HorizontalBox_PassiveSlots->AddChildToHorizontalBox(NewSlot);

        if (NewSlotSlot)
        {
            NewSlotSlot->SetPadding(FMargin(3.0f, 0.0f));
            NewSlotSlot->SetHorizontalAlignment(HAlign_Fill);
            NewSlotSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill)); 
        }

        if (i < VisibleCount)
        {
            NewSlot->SetVisibility(ESlateVisibility::Visible);

            // if (PassiveArray.IsValidIndex(i) && PassiveArray[i]->Icon)
            // {
            //     NewSlot->SetPassiveIcon(PassiveArray[i]->Icon);
            // }
        }
        else
        {
            NewSlot->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}
