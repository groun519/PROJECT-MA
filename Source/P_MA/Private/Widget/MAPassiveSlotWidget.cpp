// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/MAPassiveSlotWidget.h"
#include "Components/Image.h"

void UMAPassiveSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UMAPassiveSlotWidget::SetPassiveIcon(UTexture2D* IconTexture)
{
    if (PassiveIcon && IconTexture)
    {
        PassiveIcon->SetBrushFromTexture(IconTexture);
    }
}
