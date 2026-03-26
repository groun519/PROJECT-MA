// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsSectionHeaderWidget.h"

#include "Components/TextBlock.h"

void USettingsSectionHeaderWidget::SetupHeader(const FText& InText)
{
	HeaderText->SetText(InText);
}
