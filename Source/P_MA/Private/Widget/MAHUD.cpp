#include "Widget/MAHUD.h"
#include "Widget/MASkillSlotWidget.h"
#include "Widget/HealthBarWidget.h"
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