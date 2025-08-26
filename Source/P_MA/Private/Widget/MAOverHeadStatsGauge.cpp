// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAOverHeadStatsGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widget/MAValueGauge.h"
#include "GAS/MAAttributeSet.h"

void UMAOverHeadStatsGauge::ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (AbilitySystemComponent)
	{
		HealthBar->SetAndBoundToGameplayAttribute(AbilitySystemComponent, UMAAttributeSet::GetHealthAttribute(), UMAAttributeSet::GetMaxHealthAttribute());
	}
}
