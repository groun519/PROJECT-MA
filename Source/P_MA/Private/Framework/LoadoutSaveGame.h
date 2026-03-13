// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "LoadoutSaveGame.generated.h"

UCLASS()
class P_MA_API ULoadoutSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	FLoadoutSelection SavedLoadout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loadout")
	// Reserved for future save migration. Not used in current load path.
	int32 Version = 1;
};
