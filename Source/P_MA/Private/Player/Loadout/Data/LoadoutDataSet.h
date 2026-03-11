// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LoadoutDataSet.generated.h"

class UDataTable;
class ULoadoutEyeColorPresetData;
class ULoadoutBodyColorPresetData;

UCLASS(BlueprintType)
class P_MA_API ULoadoutDataSet : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<UDataTable> MountDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<UDataTable> WeaponDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<UDataTable> EyeShapeDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<ULoadoutEyeColorPresetData> EyeColorPreset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout")
	TObjectPtr<ULoadoutBodyColorPresetData> BodyColorPreset;
};
