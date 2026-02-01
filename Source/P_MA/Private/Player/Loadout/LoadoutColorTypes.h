// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LoadoutColorTypes.generated.h"

USTRUCT(BlueprintType)
struct FMaterialParamData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Emissive = 0.f;
};

USTRUCT(BlueprintType)
struct FMaterialParamDataPair
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMaterialParamData BodyData = FMaterialParamData();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMaterialParamData EyeData = FMaterialParamData();
};
