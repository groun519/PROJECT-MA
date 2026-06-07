// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/StatsGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"


void UStatsGauge::NativePreConstruct()
{
	Super::NativePreConstruct();
	Icon->SetBrushFromMaterial(IconMaterial);
}

void UStatsGauge::NativeConstruct()
{
	Super::NativeConstruct();
	NumberFormattingOptions.MaximumFractionalDigits = 0;
	APawn* OwnerPlayerPawn = GetOwningPlayerPawn();
	if (!OwnerPlayerPawn)
		return;

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerPlayerPawn);

	if (OwnerASC)
	{
		bool bFound;
		float AttributeVal = OwnerASC->GetGameplayAttributeValue(Attribute, bFound);
		SetValue(AttributeVal);

		OwnerASC->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UStatsGauge::AttributeChanged);
	}
}

void UStatsGauge::SetValue(float NewVal)
{
	AttributeText->SetText(FText::AsNumber(NewVal, &NumberFormattingOptions));
}

void UStatsGauge::AttributeChanged(const FOnAttributeChangeData& Data)
{
	SetValue(Data.NewValue);
}