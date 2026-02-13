// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Loop/LoopPlayerStatusWidget.h"
#include "Components/Image.h"

void ULoopPlayerStatusWidget::SetStatus(const FLoopReadyPlayerStatus& Status)
{
	if (BackgroundImage)
	{
		BackgroundImage->SetColorAndOpacity(Status.bReady ? ReadyBackgroundColor : WaitingBackgroundColor);
	}

	if (BodyColorImage)
	{
		BodyColorImage->SetColorAndOpacity(Status.BodyColor);
	}

	if (EyeColorImage)
	{
		EyeColorImage->SetColorAndOpacity(Status.EyeColor);
	}
}
