#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MAElementalConfigData.generated.h"

UCLASS(BlueprintType)
class P_MA_API UMAElementalConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category="Frozen")
	float FrozenEnterTemperature = -100.f;

	UPROPERTY(EditDefaultsOnly, Category="Frozen")
	float FrozenExitTemperature = -80.f;

	UPROPERTY(EditDefaultsOnly, Category="Frozen", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FrozenSlowMinMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Burn", meta=(ClampMin="0.0"))
	float MaxBurnDamagePerTick = 5.f;
};
