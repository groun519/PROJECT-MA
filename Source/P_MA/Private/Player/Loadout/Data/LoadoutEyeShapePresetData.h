// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "LoadoutEyeShapePresetData.generated.h"

class UMaterialInterface;
class UTexture2D;

UENUM(BlueprintType)
enum class ELoadoutEyeShapeMode : uint8
{
	Custom UMETA(DisplayName = "Custom"),
	Texture UMETA(DisplayName = "Texture")
};

USTRUCT(BlueprintType)
struct FLoadoutEyeShapeData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head")
	ELoadoutEyeShapeMode ShapeMode = ELoadoutEyeShapeMode::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head", meta=(DisplayName="Radius_Inner", EditCondition="ShapeMode==ELoadoutEyeShapeMode::Custom"))
	float RadiusInner = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head", meta=(DisplayName="Radius_Outter"))
	float RadiusOutter = 0.045f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head", meta=(DisplayName="Softness", EditCondition="ShapeMode==ELoadoutEyeShapeMode::Custom"))
	float Softness = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head", meta=(DisplayName="Eye_Width"))
	float EyeWidth = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head", meta=(DisplayName="Eye_Height"))
	float EyeHeight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head", meta=(DisplayName="_EyeTexture", EditCondition="ShapeMode==ELoadoutEyeShapeMode::Texture"))
	TSoftObjectPtr<UTexture2D> EyeTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loadout|Head")
	TSoftObjectPtr<UMaterialInterface> IconMaterial;
};

namespace LoadoutEyeShapeTableUtils
{
	inline bool ResolveEyeShapeData(const UDataTable* DataTable, FName EyeShapeId, FEyeShapeParamData& OutEyeShapeData)
	{
		if (!DataTable || EyeShapeId.IsNone())
		{
			return false;
		}

		const FLoadoutEyeShapeData* Row = DataTable->FindRow<FLoadoutEyeShapeData>(EyeShapeId, TEXT("ResolveEyeShapeData"));
		if (!Row)
		{
			return false;
		}

		OutEyeShapeData.RadiusInner = Row->RadiusInner;
		OutEyeShapeData.RadiusOutter = Row->RadiusOutter;
		OutEyeShapeData.Softness = Row->Softness;
		OutEyeShapeData.EyeWidth = Row->EyeWidth;
		OutEyeShapeData.EyeHeight = Row->EyeHeight;
		OutEyeShapeData.UseTexture = (Row->ShapeMode == ELoadoutEyeShapeMode::Texture) ? 1.f : 0.f;
		OutEyeShapeData.EyeTexture = OutEyeShapeData.UseTexture > 0.5f ? Row->EyeTexture.LoadSynchronous() : nullptr;
		return true;
	}
}
