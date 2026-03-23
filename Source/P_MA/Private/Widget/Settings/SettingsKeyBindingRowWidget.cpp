// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsKeyBindingRowWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void USettingsKeyBindingRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CurrentKeyButton->OnClicked.AddUniqueDynamic(this, &USettingsKeyBindingRowWidget::HandleCurrentKeyButtonClicked);
	if (SecondaryKeyButton) SecondaryKeyButton->OnClicked.AddUniqueDynamic(this, &USettingsKeyBindingRowWidget::HandleSecondaryKeyButtonClicked);
	ResetButton->OnClicked.AddUniqueDynamic(this, &USettingsKeyBindingRowWidget::HandleResetButtonClicked);
}

void USettingsKeyBindingRowWidget::SetupRow(const FText& InActionName, const FText& InKeyText)
{
	ActionNameText->SetText(InActionName);
	SetKeyText(CurrentKeyText, InKeyText);
	SetSecondaryKeyVisible(false);
}

void USettingsKeyBindingRowWidget::SetupSecondaryKey(const FText& InKeyText)
{
	if (!SecondaryKeyText) return;

	SetKeyText(SecondaryKeyText, InKeyText);
	SetSecondaryKeyVisible(true);
}

void USettingsKeyBindingRowWidget::SetKeyTextBySlot(int32 SlotIndex, const FText& InKeyText)
{
	if (SlotIndex == 1)
	{
		if (!SecondaryKeyText) return;

		SetKeyText(SecondaryKeyText, InKeyText);
		SetSecondaryKeyVisible(true);
		return;
	}

	SetKeyText(CurrentKeyText, InKeyText);
}

void USettingsKeyBindingRowWidget::SetKeyText(UTextBlock* TargetText, const FText& InKeyText)
{
	TargetText->SetText(InKeyText);
	TargetText->SetVisibility(InKeyText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
}

void USettingsKeyBindingRowWidget::SetSecondaryKeyVisible(bool bVisible)
{
	if (!SecondaryKeyButton) return;

	SecondaryKeyButton->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void USettingsKeyBindingRowWidget::HandleCurrentKeyButtonClicked()
{
	OnRebindRequested.Broadcast(this, 0);
}

void USettingsKeyBindingRowWidget::HandleSecondaryKeyButtonClicked()
{
	OnRebindRequested.Broadcast(this, 1);
}

void USettingsKeyBindingRowWidget::HandleResetButtonClicked()
{
	OnResetRequested.Broadcast(this);
}
