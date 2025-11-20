// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/PA_AbilitySystemGenerics.h"

#include "GameplayTagContainer.h"

UUtilityModule* UPA_AbilitySystemGenerics::FindSkillUtilityModuleByTag(const FGameplayTag& UtilityTag) const
{
	if (UtilityTag.IsValid())
	{
		return SkillUtilityModules.FindRef(UtilityTag);
	}
	return nullptr;
}
