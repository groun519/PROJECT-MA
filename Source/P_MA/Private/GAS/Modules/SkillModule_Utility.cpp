// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Utility.h"

#include "GameplayEffect.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"

void USkillModule_Utility::ModifyDamageSpec(FGameplayEffectSpecHandle& SpecHandle) const
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill || !SpecHandle.IsValid())
		return;

	const FModuleUtilityData& UtilityData = Skill->GetUtilityData();
	
	SpecHandle.Data->SetSetByCallerMagnitude(DamageModTag, UtilityData.DamageMultiplier);
}

void USkillModule_Utility::ModifyCooldownSpec(FGameplayEffectSpecHandle& SpecHandle) const
{
	if (SpecHandle.IsValid())
	{
		float Current = SpecHandle.Data->GetSetByCallerMagnitude(CooldownModTag, false, 1.0f);
		SpecHandle.Data->SetSetByCallerMagnitude(CooldownModTag, Current * CooldownMultiplier);
	}
}

float USkillModule_Utility::GetAnimSpeedMultiplier() const
{
	if (UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill))
	{
		return Skill->GetUtilityData().MontagePlayRate;
	}
	return 1.f;
}
