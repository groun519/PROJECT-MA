// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Loading/LoadingPlayerStatusWidget.h"
#include "Components/Image.h"

void ULoadingPlayerStatusWidget::SetStatus(const FLoadingPlayerStatus& Status)
{
	if (BackgroundImage)
	{
		BackgroundImage->SetColorAndOpacity(Status.bLoaded ? LoadedBackgroundColor : WaitingBackgroundColor);
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
