// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Elemental.h"

#include "GameplayEffect.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"


void USkillModule_Elemental::CreateAdditionalEffectSpecs(TArray<FGameplayEffectSpecHandle>& OutAdditionalSpecs) const
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill)	return;

	const FModuleElementalData& ElementData = Skill->GetElementalData();
	if (ElementData.AdditionalEffect)
	{
		FGameplayEffectSpecHandle Spec = Skill->MakeOutgoingGameplayEffectSpec(ElementData.AdditionalEffect, Skill->GetAbilityLevel());
		if (Spec.IsValid())
		{
			OutAdditionalSpecs.Add(Spec);
		}
	}
}
