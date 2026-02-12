// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LoadoutSaveGame.generated.h"

UCLASS()
class P_MA_API ULoadoutSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	FMaterialParamDataPair SavedColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	FName SavedWeaponId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	int32 Version = 1;
};
