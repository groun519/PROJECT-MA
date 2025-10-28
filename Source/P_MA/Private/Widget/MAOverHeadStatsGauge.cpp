// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAOverHeadStatsGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widget/MAValueGauge.h"
#include "GAS/MAAttributeSet.h"

void UMAOverHeadStatsGauge::ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	if (!AbilitySystemComponent)
		return;

	if (HealthBar)
	{
		HealthBar->SetAndBoundToGameplayAttribute(
			AbilitySystemComponent,
			UMAAttributeSet::GetHealthAttribute(),
			UMAAttributeSet::GetMaxHealthAttribute());
	}

	// FuryBar는 선택적 바인딩 (몬스터만 존재)
	if (FuryBar)
	{
		FuryBar->SetAndBoundToGameplayAttribute(
			AbilitySystemComponent,
			UMAAttributeSet::GetFuryAttribute(),
			UMAAttributeSet::GetMaxFuryAttribute());
	}
}
