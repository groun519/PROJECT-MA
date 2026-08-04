#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "MAGameplayEffect_MonsterWaveStatScale.generated.h"

class UAbilitySystemComponent;

UCLASS()
class P_MA_API UMAGameplayEffect_MonsterWaveStatScale : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMAGameplayEffect_MonsterWaveStatScale();

	static FName GetStatCoefficientDataName();
	static void ApplyTo(UAbilitySystemComponent& AbilitySystemComponent, float StatCoefficient);
};
