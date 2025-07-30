// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthBarWidget.h"
#include "Components/ProgressBar.h"

void UHealthBarWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (ProgressBar)
    {
        ProgressBar->SetFillColorAndOpacity(BarColor);
    }
}

void UHealthBarWidget::SetHealthPercent(float HealthPercent)
{
    if (ProgressBar)
    {
        ProgressBar->SetPercent(FMath::Clamp(HealthPercent, 0.0f, 1.0f));
    }
}
