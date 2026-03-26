// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsCheckboxRowWidget.h"

#include "Components/CheckBox.h"
#include "Components/TextBlock.h"

void USettingsCheckboxRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &USettingsCheckboxRowWidget::HandleCheckStateChanged);
	RefreshState();
}

void USettingsCheckboxRowWidget::SetupState(const FText& InLabel, bool bInChecked)
{
	LabelText->SetText(InLabel);
	bChecked = bInChecked;
	RefreshState();
}

void USettingsCheckboxRowWidget::RefreshState()
{
	bUpdatingState = true;
	CheckBox->SetIsChecked(bChecked);
	bUpdatingState = false;
}

void USettingsCheckboxRowWidget::HandleCheckStateChanged(bool bInChecked)
{
	if (bUpdatingState) return;

	bChecked = bInChecked;
	OnCheckStateChanged.Broadcast(bChecked);
}
