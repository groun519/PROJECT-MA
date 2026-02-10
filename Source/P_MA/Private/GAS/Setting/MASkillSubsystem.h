// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Modules/MASkillModuleData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MASkillSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class UMASkillSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	const FSkillData* GetSkillData(FName SkillID);

	const FModuleBehaviorData* GetBehaviorData(FGameplayTag Tag);
	const FModuleElementalData* GetElementalData(FGameplayTag Tag);
	const FModuleUtilityData* GetUtilityData(FGameplayTag Tag);

private:
	UPROPERTY()		UDataTable* LoadedMasterTable;
	UPROPERTY()		UDataTable* LoadedBehaviorTable;
	UPROPERTY()		UDataTable* LoadedElementalTable;
	UPROPERTY()		UDataTable* LoadedUtilityTable;
};
