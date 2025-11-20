// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/PA_AbilitySystemGenerics.h"

#include "GameplayTagContainer.h"
#include "GameplayTagsManager.h"
#include "MAGameplayAbilityTypes.h"

UUtilityModule* UPA_AbilitySystemGenerics::FindSkillUtilityModuleByTag(const FGameplayTag& UtilityTag, UObject* Outer) const
{
	if (!UtilityModuleDataTable || !UtilityTag.IsValid())
		return nullptr;
	
	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(UtilityTag, TagNames);
	FName UtilityName = TagNames.Last();

	FSkillUtilityModule* DTRow = UtilityModuleDataTable->FindRow<FSkillUtilityModule>(UtilityName,TEXT(""));
	if (!DTRow)
		return nullptr;
	
	UUtilityModule* NewModule = NewObject<UUtilityModule>(Outer);
	NewModule->InitFromData(*DTRow);
	return NewModule;
}
