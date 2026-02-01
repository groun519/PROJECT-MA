// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LoadoutEyeColorPresetData.generated.h"

UCLASS(BlueprintType)
class P_MA_API ULoadoutEyeColorPresetData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head")
	TArray<FLinearColor> EyeColors;
};
