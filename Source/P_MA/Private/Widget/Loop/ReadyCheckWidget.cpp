// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Loop/ReadyCheckWidget.h"

#include "Components/Image.h"

void UReadyCheckWidget::SetReadyState(bool bReady)
{
	if (ReadyOnImage)
	{
		ReadyOnImage->SetVisibility(bReady ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (ReadyOffImage)
	{
		ReadyOffImage->SetVisibility(bReady ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}
