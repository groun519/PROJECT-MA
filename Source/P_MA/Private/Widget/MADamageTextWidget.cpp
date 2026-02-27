// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MADamageTextWidget.h"

#include "Components/TextBlock.h"

void UMADamageTextWidget::SetDamageText(float DamageAmount, bool bIsCritical)
{
	if (DamageText)
	{
		FString DamageString = FString::Printf(TEXT("%d"), FMath::RoundToInt(DamageAmount));
		DamageText->SetText(FText::FromString(DamageString));
		
		if (bIsCritical)
		{
			DamageText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
		}else
		{
			DamageText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}
	if (FadeUpAnim)
	{
		PlayAnimation(FadeUpAnim);
	}
}
