// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MASkillSetting.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta=(DisplayName = "Skill System Setting"))
class UMASkillSetting : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	static const UMASkillSetting* Get() {return GetDefault<UMASkillSetting>();}

	UPROPERTY(Config, EditAnywhere, Category="Data Tables")
	TSoftObjectPtr<UDataTable> SkillMasterTable;
	
	UPROPERTY(Config, EditAnywhere, Category="Data Tables")
	TSoftObjectPtr<UDataTable> UtilityTable;
};
