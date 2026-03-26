// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsToggleRowWidget.h"

#include "Components/TextBlock.h"
#include "Widget/Settings/SettingsToggleButtonWidget.h"

void USettingsToggleRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	CacheToggleButtons();
	for (USettingsToggleButtonWidget* Button : ToggleButtons)
	{
		Button->OnClicked.AddUObject(this, &USettingsToggleRowWidget::HandleToggleClicked);
	}
}

void USettingsToggleRowWidget::SetupOptions(const FText& InLabel, const TArray<FText>& InOptions, int32 InSelectedIndex)
{
	LabelText->SetText(InLabel);

	const int32 UseCount = FMath::Min(ToggleButtons.Num(), InOptions.Num());
	ActiveButtonCount = UseCount;
	SelectedIndex = FMath::Clamp(InSelectedIndex, 0, FMath::Max(0, UseCount - 1));

	for (int32 Index = 0; Index < ToggleButtons.Num(); ++Index)
	{
		USettingsToggleButtonWidget* Button = ToggleButtons[Index];

		Button->SetButtonIndex(Index);
		if (Index < UseCount)
		{
			Button->SetLabel(InOptions[Index]);
			Button->SetIsEnabled(true);
		}
		else
		{
			Button->SetLabel(FText::GetEmpty());
			Button->SetIsEnabled(false);
		}
	}

	UpdateSelection();
}

void USettingsToggleRowWidget::CacheToggleButtons()
{
	ToggleButtons.Reserve(5);
	if (ToggleButton_0) ToggleButtons.Add(ToggleButton_0);
	if (ToggleButton_1) ToggleButtons.Add(ToggleButton_1);
	if (ToggleButton_2) ToggleButtons.Add(ToggleButton_2);
	if (ToggleButton_3) ToggleButtons.Add(ToggleButton_3);
	if (ToggleButton_4) ToggleButtons.Add(ToggleButton_4);
}

void USettingsToggleRowWidget::UpdateSelection()
{
	bUpdatingSelection = true;

	for (int32 Index = 0; Index < ToggleButtons.Num(); ++Index)
	{
		ToggleButtons[Index]->SetSelected(Index == SelectedIndex);
	}

	bUpdatingSelection = false;
}

void USettingsToggleRowWidget::HandleToggleClicked(int32 InIndex)
{
	if (bUpdatingSelection) return;
	if (ActiveButtonCount <= 0) return;
	if (InIndex < 0 || InIndex >= ActiveButtonCount) return;

	SelectedIndex = InIndex;
	UpdateSelection();
	OnSelectionChanged.Broadcast(SelectedIndex);
}
