#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MAModuleQualityData.generated.h"

UENUM(BlueprintType)
enum class EMAModuleGrade : uint8
{
	Grade1,
	Grade2,
	Grade3,
	Grade4,
	Grade5,
	Grade6
};

UENUM(BlueprintType)
enum class EMAModuleRarity : uint8
{
	Rarity1,
	Rarity2,
	Rarity3,
	Rarity4,
	Rarity5,
	Rarity6,
	Rarity7
};

USTRUCT(BlueprintType)
struct FMAModuleQuality
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Module|Quality")
	EMAModuleGrade Grade = EMAModuleGrade::Grade1;

	UPROPERTY(EditDefaultsOnly, Category="Module|Quality")
	EMAModuleRarity Rarity = EMAModuleRarity::Rarity4;
};

USTRUCT(BlueprintType)
struct FMAModuleGradeData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Grade")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category="Grade")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Price", meta=(ClampMin="0"))
	int32 BasePrice = 0;
};

USTRUCT(BlueprintType)
struct FMAModuleRarityData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Rarity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category="Rarity")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Rarity", meta=(ClampMin="0"))
	float SelectionWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Price")
	int32 PriceOffset = 0;
};

UCLASS(BlueprintType)
class P_MA_API UMAModuleQualityData : public UDataAsset
{
	GENERATED_BODY()

public:
	int32 ResolvePrice(const FMAModuleQuality& Quality) const;

private:
	UPROPERTY(EditDefaultsOnly, Category="Module|Quality")
	TMap<EMAModuleGrade, FMAModuleGradeData> GradeData;

	UPROPERTY(EditDefaultsOnly, Category="Module|Quality")
	TMap<EMAModuleRarity, FMAModuleRarityData> RarityData;
};
