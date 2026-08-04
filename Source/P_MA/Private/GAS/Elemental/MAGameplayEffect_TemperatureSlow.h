#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MAGameplayEffect_TemperatureSlow.generated.h"

UCLASS()
class P_MA_API UMAGameplayEffect_TemperatureSlow : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMAGameplayEffect_TemperatureSlow();

	static FName GetSlowMultiplierDataName();
};
