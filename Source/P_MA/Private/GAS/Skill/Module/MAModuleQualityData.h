#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MAModuleQualityData.generated.h"

UENUM(BlueprintType)
enum class EMAModuleType : uint8
{
	Sequence,
	Modifier,
	DebuffModifier,
	Elemental
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
	EMAModuleType Type = EMAModuleType::Modifier;

	UPROPERTY(EditDefaultsOnly, Category="Module|Quality")
	EMAModuleRarity Rarity = EMAModuleRarity::Rarity4;
};

USTRUCT(BlueprintType)
struct FMAModuleTypeData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Type")
	FLinearColor IconColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Type")
	FLinearColor InnerColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.f);
};

USTRUCT(BlueprintType)
struct FMAModuleRarityData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Rarity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, Category="Rarity")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category="Rarity", meta=(ClampMin="0.0", ClampMax="1.0"))
	float GlowAlpha = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Rarity", meta=(ClampMin="0"))
	float SelectionWeight = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Price", meta=(ClampMin="0"))
	int32 BasePrice = 0;
};

UCLASS(BlueprintType)
class P_MA_API UMAModuleQualityData : public UDataAsset
{
	GENERATED_BODY()

public:
	int32 ResolvePrice(const FMAModuleQuality& Quality) const;
	const FMAModuleRarityData* FindRarityData(EMAModuleRarity Rarity) const { return RarityData.Find(Rarity); }
	const FMAModuleTypeData* FindTypeData(EMAModuleType Type) const { return TypeData.Find(Type); }

private:
	UPROPERTY(EditDefaultsOnly, Category="Module|Quality")
	TMap<EMAModuleType, FMAModuleTypeData> TypeData;

	UPROPERTY(EditDefaultsOnly, Category="Module|Quality")
	TMap<EMAModuleRarity, FMAModuleRarityData> RarityData;
};
