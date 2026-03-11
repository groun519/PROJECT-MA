// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LoadoutColorTypes.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct FMaterialParamData
{
	GENERATED_BODY()

	FMaterialParamData() = default;
	explicit FMaterialParamData(const FLinearColor& InColor)
		: Color(InColor)
	{
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FLinearColor Color = FLinearColor(0.f, 0.f, 0.f, 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Emissive = 0.f;
};

USTRUCT(BlueprintType)
struct FMaterialParamDataPair
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMaterialParamData BodyData = FMaterialParamData(FLinearColor(0.25f, 0.25f, 0.25f, 1.0f));

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FMaterialParamData EyeData = FMaterialParamData(FLinearColor::White);
};

USTRUCT(BlueprintType)
struct FEyeShapeParamData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RadiusInner = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float RadiusOutter = 0.045f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Softness = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float EyeWidth = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float EyeHeight = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float UseTexture = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UTexture2D> EyeTexture = nullptr;
};
