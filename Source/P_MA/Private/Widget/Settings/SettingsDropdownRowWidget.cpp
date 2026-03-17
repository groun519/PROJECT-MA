// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsDropdownRowWidget.h"

#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"

void USettingsDropdownRowWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Dropdown->OnGenerateWidgetEvent.BindUFunction(this, FName("HandleGenerateWidget"));
	Dropdown->OnSelectionChanged.AddUniqueDynamic(this, &USettingsDropdownRowWidget::HandleSelectionChanged);
	RefreshOptions();
}

void USettingsDropdownRowWidget::SetupOptions(const FText& InLabel, const TArray<FText>& InOptions, int32 InSelectedIndex)
{
	LabelText->SetText(InLabel);
	Options = InOptions;
	SelectedIndex = Options.IsValidIndex(InSelectedIndex) ? InSelectedIndex : INDEX_NONE;
	if (SelectedIndex == INDEX_NONE && Options.Num() > 0)
	{
		SelectedIndex = 0;
	}
	RefreshOptions();
}

void USettingsDropdownRowWidget::SetOptionEnabledFlags(const TArray<bool>& InEnabledFlags)
{
	OptionEnabledFlags = InEnabledFlags;
	if (OptionEnabledFlags.Num() != Options.Num())
	{
		OptionEnabledFlags.Init(true, Options.Num());
	}
	if (!OptionEnabledFlags.Contains(true))
	{
		OptionEnabledFlags.Init(true, Options.Num());
	}

	if (OptionEnabledFlags.IsValidIndex(SelectedIndex) && !OptionEnabledFlags[SelectedIndex])
	{
		SelectedIndex = OptionEnabledFlags.IndexOfByKey(true);
		if (SelectedIndex == INDEX_NONE && Options.Num() > 0)
		{
			SelectedIndex = 0;
		}
	}
	if (SelectedIndex == INDEX_NONE)
	{
		SelectedIndex = OptionEnabledFlags.IndexOfByKey(true);
		if (SelectedIndex == INDEX_NONE && Options.Num() > 0)
		{
			SelectedIndex = 0;
		}
	}

	RefreshOptions();
}

void USettingsDropdownRowWidget::RefreshOptions()
{
	Dropdown->ClearOptions();

	for (const FText& Option : Options)
	{
		Dropdown->AddOption(Option.ToString());
	}

	if (!Options.IsValidIndex(SelectedIndex)) return;

	bUpdatingSelection = true;
	Dropdown->SetSelectedOption(Options[SelectedIndex].ToString());
	bUpdatingSelection = false;
}

UWidget* USettingsDropdownRowWidget::HandleGenerateWidget(FString Item)
{
	UTextBlock* TextWidget = NewObject<UTextBlock>(this);
	TextWidget->SetText(FText::FromString(Item));
	TextWidget->SetJustification(ETextJustify::Center);

	const FText ItemText = FText::FromString(Item);
	const int32 OptionIndex = Options.IndexOfByPredicate([&ItemText](const FText& Option)
	{
		return Option.EqualTo(ItemText);
	});

	const bool bOptionEnabled = !OptionEnabledFlags.IsValidIndex(OptionIndex) || OptionEnabledFlags[OptionIndex];
	TextWidget->SetColorAndOpacity(bOptionEnabled ? FSlateColor::UseForeground() : FSlateColor(FLinearColor(0.5f, 0.5f, 0.5f)));
	return TextWidget;
}

void USettingsDropdownRowWidget::HandleSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (bUpdatingSelection) return;

	const int32 PreviousIndex = SelectedIndex;
	const FText SelectedText = FText::FromString(SelectedItem);
	SelectedIndex = Options.IndexOfByPredicate([&SelectedText](const FText& Option)
	{
		return Option.EqualTo(SelectedText);
	});

	if (OptionEnabledFlags.IsValidIndex(SelectedIndex) && !OptionEnabledFlags[SelectedIndex])
	{
		int32 FallbackIndex = PreviousIndex;
		if (!OptionEnabledFlags.IsValidIndex(FallbackIndex) || !OptionEnabledFlags[FallbackIndex])
		{
			FallbackIndex = OptionEnabledFlags.IndexOfByKey(true);
		}

		bUpdatingSelection = true;
		if (Options.IsValidIndex(FallbackIndex))
		{
			Dropdown->SetSelectedOption(Options[FallbackIndex].ToString());
			SelectedIndex = FallbackIndex;
		}
		bUpdatingSelection = false;
		return;
	}

	if (SelectedIndex != INDEX_NONE) OnSelectionChanged.Broadcast(SelectedIndex);
}
