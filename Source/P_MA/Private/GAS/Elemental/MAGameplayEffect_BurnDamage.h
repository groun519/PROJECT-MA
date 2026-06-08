#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"
#include "MAGameplayEffect_BurnDamage.generated.h"

UCLASS()
class P_MA_API UExecCalc_BurnDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_BurnDamage();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	FGameplayEffectAttributeCaptureDefinition TargetTemperatureDef;
};

UCLASS()
class P_MA_API UMAGameplayEffect_BurnDamage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMAGameplayEffect_BurnDamage();

	static FName GetMaxBurnDamageDataName();
};
