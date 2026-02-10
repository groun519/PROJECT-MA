// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/MASkillModule.h"

#include "MASkillModuleData.h"

void UMASkillModule::ApplyModuleToSkillData(FSkillData& OutSkillData, const FModuleBehaviorData& ModuleData) const
{
	if (!ModuleData.ActionTagOverride.IsEmpty())
	{
		if (ModuleData.bReplaceActionTags)
			OutSkillData.ActionTags = ModuleData.ActionTagOverride;
		else
		{
			OutSkillData.ActionTags.AppendTags(ModuleData.ActionTagOverride);
		}
	}

	if (ModuleData.ActionDataOverride.IsValid())
	{
		OutSkillData.ActionData = ModuleData.ActionDataOverride;
	}
}
