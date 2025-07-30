#include "MAHUD.h"
#include "HealthBarWidget.h"
#include "MAAttackSlotWidget.h" 
#include "MASkillSlotWidget.h" 
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
    if (!AttackSlotWidgetClass || !HorizontalBox_SkillSlots) return;

    for (int32 i = 0; i < NumSlots; ++i)
    {
        UMAAttackSlotWidget* NewSlot = CreateWidget<UMAAttackSlotWidget>(GetWorld(), AttackSlotWidgetClass);
        if (!NewSlot)
        {
            UE_LOG(LogTemp, Error, TEXT("NewSlot is nullptr at index %d!"), i);
            continue;
        }

        FString KeyName = (i == 0) ? TEXT("Attack") : TEXT("Skill");
        UE_LOG(LogTemp, Warning, TEXT("Calling SetHotkeyName(%s)"), *KeyName); 
        NewSlot->SetHotkeyName(KeyName);

        HorizontalBox_SkillSlots->AddChildToHorizontalBox(NewSlot);
    }

}