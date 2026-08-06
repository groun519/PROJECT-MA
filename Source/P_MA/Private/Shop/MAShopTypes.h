#pragma once

#include "CoreMinimal.h"
#include "MAShopTypes.generated.h"

class UMASkillModule;

USTRUCT(BlueprintType)
struct FMAShopProduct
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int32 StockId = INDEX_NONE;

	UPROPERTY(Transient)
	int32 VisualSeed = 0;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModule> Module = nullptr;

	UPROPERTY(Transient)
	int32 Price = 0;
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
