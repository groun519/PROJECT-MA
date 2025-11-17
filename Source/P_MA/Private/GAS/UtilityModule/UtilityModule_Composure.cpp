// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UtilityModule/UtilityModule_Composure.h"

#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

UUtilityModule_Composure::UUtilityModule_Composure()
{
}

float UUtilityModule_Composure::ModifyCooldownDuration(float OriginalDuration) const
{
	return OriginalDuration * CooldownMultiplier;
}

void UUtilityModule_Composure::ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DamageModifierTag, DamagePercentAdditive);
	}
}
