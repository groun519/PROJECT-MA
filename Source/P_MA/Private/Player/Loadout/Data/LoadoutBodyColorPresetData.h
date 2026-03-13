// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "LoadoutBodyColorPresetData.generated.h"

UCLASS(BlueprintType)
class P_MA_API ULoadoutBodyColorPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Body")
	TArray<FMaterialParamData> BodyColors;
};
