// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/UtilityModule/UtilityModule_Smite.h"

#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"

void UUtilityModule_Smite::ModifyDamageEffectSpec(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid())
	{
		//MMC는 태그를 찾아 값을 읽어감
		SpecHandle.Data->SetSetByCallerMagnitude(DamageModifierTag, DamagePercentAdditive);
	}
}
