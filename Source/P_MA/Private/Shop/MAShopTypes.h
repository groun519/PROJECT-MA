#pragma once

#include "CoreMinimal.h"
#include "MAShopTypes.generated.h"

class UMASkillDefinition;
class UTexture2D;

USTRUCT(BlueprintType)
struct FMAShopStockEntry
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int32 StockId = INDEX_NONE;

	UPROPERTY(Transient)
	int32 VisualSeed = 0;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillDefinition> SkillDefinition = nullptr;

	UPROPERTY(Transient)
	int32 Price = 0;

	UPROPERTY(Transient)
	FText QualityText;

	UPROPERTY(Transient)
	FLinearColor QualityColor = FLinearColor::White;

	UPROPERTY(Transient)
	float GlowAlpha = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(Transient)
	FLinearColor IconColor = FLinearColor::White;

	UPROPERTY(Transient)
	FLinearColor InnerColor = FLinearColor(0.15f, 0.15f, 0.15f, 1.f);
};

USTRUCT(BlueprintType)
struct FMAShopStockCountRange
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Shop|Stock", meta=(ClampMin="0"))
	int32 Min = 0;

	UPROPERTY(EditDefaultsOnly, Category="Shop|Stock", meta=(ClampMin="0"))
	int32 Max = 0;

	int32 ResolveCount() const
	{
		const int32 ClampedMin = FMath::Max(0, Min);
		const int32 ClampedMax = FMath::Max(ClampedMin, Max);
		return ClampedMin == ClampedMax ? ClampedMin : FMath::RandRange(ClampedMin, ClampedMax);
	}
};
