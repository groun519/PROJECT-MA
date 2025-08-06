// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MASkillSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"


void UMASkillSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();


}

void UMASkillSlotWidget::SetHotkeyName(const FString& KeyName)
{
    if (HotKeyName)
    {
        HotKeyName->SetText(FText::FromString(KeyName));
    }
    if (!HotKeyName)
    {
        UE_LOG(LogTemp, Error, TEXT("[SlotWidget] HotKeyName is nullptr!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[SlotWidget] Setting HotKeyName to: %s"), *KeyName);
    HotKeyName->SetText(FText::FromString(KeyName));
}

void UMASkillSlotWidget::SetSkillIcon(UTexture2D* IconTexture)
{
    if (SkillSlot && IconTexture)
    {
        SkillSlot->SetBrushFromTexture(IconTexture);
    }

}