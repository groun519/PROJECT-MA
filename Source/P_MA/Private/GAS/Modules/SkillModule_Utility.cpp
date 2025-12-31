// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Utility.h"

#include "GameplayEffect.h"

void USkillModule_Utility::ModifyDamageSpec(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid())
	{
		float Current = SpecHandle.Data->GetSetByCallerMagnitude(DamageModTag,false,1.f);
		SpecHandle.Data->SetSetByCallerMagnitude(DamageModTag,DamageMultiplier*Current);
	}
}

void USkillModule_Utility::ModifyCooldownSpec(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid())
	{
		float Current = SpecHandle.Data->GetSetByCallerMagnitude(CooldownModTag, false, 1.0f);
		SpecHandle.Data->SetSetByCallerMagnitude(CooldownModTag, Current * CooldownMultiplier);
	}
}
