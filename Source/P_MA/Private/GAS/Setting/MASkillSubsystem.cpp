// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Setting/MASkillSubsystem.h"

#include "MASkillSetting.h"

const FSkillData* UMASkillSubsystem::GetSkillData(FName SkillID)
{
	if (!LoadedMasterTable)
	{
		LoadedMasterTable = UMASkillSetting::Get()->SkillMasterTable.LoadSynchronous();
	}
	if (LoadedMasterTable && !SkillID.IsNone())
	{
		return LoadedMasterTable->FindRow<FSkillData>(SkillID,"");
	}
	return nullptr;
}

const FModuleUtilityData* UMASkillSubsystem::GetUtilityData(FGameplayTag Tag)
{
	if (!LoadedUtilityTable)
	{
		LoadedUtilityTable = UMASkillSetting::Get()->UtilityTable.LoadSynchronous();
	}
	if (LoadedUtilityTable && Tag.IsValid())
	{
		return LoadedUtilityTable->FindRow<FModuleUtilityData>(Tag.GetTagName(),"");
	}
	return nullptr;
}
