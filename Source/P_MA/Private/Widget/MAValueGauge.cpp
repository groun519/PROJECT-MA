// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAValueGauge.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "Components/TextBlock.h"

void UMAValueGauge::NativePreConstruct()
{
	Super::NativePreConstruct();
	ProgressBar->SetFillColorAndOpacity(BarColor);

	ValueText->SetFont(ValueTextFont);

	ValueText->SetVisibility(bValueTextVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	ProgressBar->SetVisibility(bProgressBarVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UMAValueGauge::SetAndBoundToGameplayAttribute(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute)
{
	if (AbilitySystemComponent)
	{
		bool bFound;
		float Value = AbilitySystemComponent->GetGameplayAttributeValue(Attribute, bFound);
		float MaxValue = AbilitySystemComponent->GetGameplayAttributeValue(MaxAttribute, bFound);
		if (bFound)
		{
			SetValue(Value, MaxValue);
		}

		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UMAValueGauge::ValueChanged);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UMAValueGauge::MaxValueChanged);
	}
}

void UMAValueGauge::SetValue(float NewValue, float NewMaxValue)
{
	CachedValue = NewValue;
	CachedMaxValue = NewMaxValue;

	if (NewMaxValue == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Value Guage: %s, NewMaxValue can't be 0"), *GetName());
		return;
	}

	float NewPercent = NewValue / NewMaxValue;
	ProgressBar->SetPercent(NewPercent);

	FNumberFormattingOptions FormatOps = FNumberFormattingOptions().SetMaximumFractionalDigits(0);

	ValueText->SetText(
		FText::Format(
			FTextFormat::FromString("{0}/{1}"),
			FText::AsNumber(NewValue, &FormatOps),
			FText::AsNumber(NewMaxValue, &FormatOps)
		)
	);
}

void UMAValueGauge::ValueChanged(const FOnAttributeChangeData& ChangedData)
{
	SetValue(ChangedData.NewValue, CachedMaxValue);
}

void UMAValueGauge::MaxValueChanged(const FOnAttributeChangeData& ChangedData)
{
	SetValue(CachedValue, ChangedData.NewValue);
}