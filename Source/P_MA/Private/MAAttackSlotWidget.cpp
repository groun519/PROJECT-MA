// Fill out your copyright notice in the Description page of Project Settings.


#include "MAAttackSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UMAAttackSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

  
}

void UMAAttackSlotWidget::SetHotkeyName(const FString& KeyName)
{
    if (HotKeyName)
    {
        HotKeyName->SetText(FText::FromString(KeyName));
    }
}

void UMAAttackSlotWidget::SetSkillIcon(UTexture2D* IconTexture)
{
    if (AttackSlot && IconTexture)
    {
        AttackSlot->SetBrushFromTexture(IconTexture);
    }

}
