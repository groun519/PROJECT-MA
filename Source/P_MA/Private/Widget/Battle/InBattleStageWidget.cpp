// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Battle/InBattleStageWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UInBattleStageWidget::SetStageText(const FText& InText)
{
	if (StageText)
	{
		StageText->SetText(InText);
	}
}

void UInBattleStageWidget::PlayShowAnimation()
{
	if (ShowAnimation)
	{
		PlayAnimation(ShowAnimation);
	}
}

float UInBattleStageWidget::GetShowAnimationDuration() const
{
	return ShowAnimation ? (ShowAnimation->GetEndTime() - ShowAnimation->GetStartTime()) : 0.0f;
}
