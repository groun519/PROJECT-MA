// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/MASkillModule.h"

#include "MASkillModuleData.h"
#include "GAS/MAAbilitySystemStatics.h"

UMASkillModule::UMASkillModule()
{
	MeleeActionTag = UMAAbilitySystemStatics::GetMeleeActionTag();
	ProjectileActionTag = UMAAbilitySystemStatics::GetProjectileActionTag();
	TargetingActionTag = UMAAbilitySystemStatics::GetTargetingActionTag();
	MontageDamageTag = UMAAbilitySystemStatics::GetMontageDamageTag();
	MontageSpawnProjectileTag = UMAAbilitySystemStatics::GetMontageProjectileTag();
}

void UMASkillModule::ApplyModuleToSkillData(FSkillData& OutSkillData, const FModuleBehaviorData& ModuleData) const
{
	if (!ModuleData.ActionTagOverride.IsEmpty())
	{
		/*
		 *	ModuleBehaviorData에 bReplaceActionTags 변수로 설정 시 (덮어 씌울지 or 추가할지)
		if (ModuleData.bReplaceActionTags)
			OutSkillData.ActionTags = ModuleData.ActionTagOverride;
		else
		{
			OutSkillData.ActionTags.AppendTags(ModuleData.ActionTagOverride);
		}
		*/
		// 변수 사용하지 않으면 그냥 덮어씌우도록
		OutSkillData.ActionTags = ModuleData.ActionTagOverride;
	}

	if (ModuleData.ActionDataOverride.IsValid())
	{
		OutSkillData.ActionData = ModuleData.ActionDataOverride;
	}
}
