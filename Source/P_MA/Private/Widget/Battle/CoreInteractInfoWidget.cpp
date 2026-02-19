// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Battle/CoreInteractInfoWidget.h"

#include "Components/TextBlock.h"

void UCoreInteractInfoWidget::SetInfoText(const FText& InText)
{
	if (InfoText)
	{
		InfoText->SetText(InText);
	}
}
