// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UtilityModule/UtilityModule_Deliberation.h"

#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

UUtilityModule_Deliberation::UUtilityModule_Deliberation()
{
}

void UUtilityModule_Deliberation::ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DamageModifierTag, DamagePercentAdditive);
	}
}

float UUtilityModule_Deliberation::ModifyMontagePlayRate(float OriginalPlayRate) const
{
	return OriginalPlayRate * MontagePlayRate;
}
