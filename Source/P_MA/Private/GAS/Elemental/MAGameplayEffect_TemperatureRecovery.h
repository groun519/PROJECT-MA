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

	float GetRecoveryRatioPerTick() const { return RecoveryRatioPerTick; }
	float GetSnapThreshold() const { return SnapThreshold; }

private:
	UPROPERTY(EditDefaultsOnly, Category="Elemental|Temperature", meta=(ClampMin="0.0", ClampMax="1.0", AllowPrivateAccess="true"))
	float RecoveryRatioPerTick = 0.01f;

	UPROPERTY(EditDefaultsOnly, Category="Elemental|Temperature", meta=(ClampMin="0.0", AllowPrivateAccess="true"))
	float SnapThreshold = 0.1f;
};
