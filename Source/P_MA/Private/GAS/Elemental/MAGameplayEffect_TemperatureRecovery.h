#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"
#include "MAGameplayEffect_TemperatureRecovery.generated.h"

UCLASS()
class P_MA_API UExecCalc_TemperatureRecovery : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_TemperatureRecovery();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	FGameplayEffectAttributeCaptureDefinition TargetTemperatureDef;
};

UCLASS()
class P_MA_API UMAGameplayEffect_TemperatureRecovery : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UMAGameplayEffect_TemperatureRecovery();

	static FName GetRecoveryRatioDataName();
	static FName GetRecoveryAmountDataName();
};
