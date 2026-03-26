// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/Settings/SettingsSliderRowWidget.h"

#include "Components/Slider.h"
#include "Components/TextBlock.h"

void USettingsSliderRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ValueSlider->OnValueChanged.AddUniqueDynamic(this, &USettingsSliderRowWidget::HandleSliderValueChanged);
	RefreshValue();
}

void USettingsSliderRowWidget::SetupValue(const FText& InLabel, float InValue)
{
	LabelText->SetText(InLabel);
	SetValue(InValue);
}

void USettingsSliderRowWidget::SetValue(float InValue)
{
	Value = FMath::Clamp(InValue, 0.0f, 1.0f);
	RefreshValue();
}

void USettingsSliderRowWidget::RefreshValue()
{
	bUpdatingValue = true;
	ValueSlider->SetValue(Value);
	bUpdatingValue = false;

	RefreshValueText();
}

void USettingsSliderRowWidget::RefreshValueText()
{
	const int32 Percentage = FMath::RoundToInt(Value * 100.0f);
	ValueText->SetText(FText::Format(NSLOCTEXT("SettingsSliderRow", "PercentValue", "{0}%"), FText::AsNumber(Percentage)));
}

void USettingsSliderRowWidget::HandleSliderValueChanged(float InValue)
{
	if (bUpdatingValue) return;

	Value = FMath::Clamp(InValue, 0.0f, 1.0f);
	RefreshValueText();
	OnValueChanged.Broadcast(Value);
}
