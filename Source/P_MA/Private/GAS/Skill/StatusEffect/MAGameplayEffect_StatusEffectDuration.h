#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MAGameplayEffect_StatusEffectDuration.generated.h"

UCLASS()
class P_MA_API UMAGameplayEffect_StatusEffectDuration : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMAGameplayEffect_StatusEffectDuration();
};

UCLASS()
class P_MA_API UMAGameplayEffect_StatusEffectInfinite : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMAGameplayEffect_StatusEffectInfinite();
};
