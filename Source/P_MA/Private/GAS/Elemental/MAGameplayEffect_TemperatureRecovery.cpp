#include "GAS/Elemental/MAGameplayEffect_TemperatureRecovery.h"

#include "GAS/MAAttributeSet.h"

UExecCalc_TemperatureRecovery::UExecCalc_TemperatureRecovery()
{
	TargetTemperatureDef.AttributeToCapture = UMAAttributeSet::GetTemperatureAttribute();
	TargetTemperatureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	TargetTemperatureDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(TargetTemperatureDef);
}

void UExecCalc_TemperatureRecovery::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Temperature = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(TargetTemperatureDef, EvalParams, Temperature);
	if (FMath::IsNearlyZero(Temperature)) return;

	const float RecoveryRatioPerTick = Spec.GetSetByCallerMagnitude(
		UMAGameplayEffect_TemperatureRecovery::GetRecoveryRatioDataName(),
		false,
		0.01f);
	const float RecoveryAmountPerTick = Spec.GetSetByCallerMagnitude(
		UMAGameplayEffect_TemperatureRecovery::GetRecoveryAmountDataName(),
		false,
		0.1f);
	const float AbsTemperature = FMath::Abs(Temperature);
	const float RecoveryAmount = AbsTemperature * RecoveryRatioPerTick + RecoveryAmountPerTick;
	const float Delta = -FMath::Sign(Temperature) * FMath::Min(AbsTemperature, RecoveryAmount);
	if (FMath::IsNearlyZero(Delta)) return;

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UMAAttributeSet::GetTemperatureAttribute(),
		EGameplayModOp::Additive,
		Delta));
}

UMAGameplayEffect_TemperatureRecovery::UMAGameplayEffect_TemperatureRecovery()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = FScalableFloat(0.1f);
	bExecutePeriodicEffectOnApplication = false;

	FGameplayEffectExecutionDefinition& ExecutionDefinition = Executions.AddDefaulted_GetRef();
	ExecutionDefinition.CalculationClass = UExecCalc_TemperatureRecovery::StaticClass();
}

FName UMAGameplayEffect_TemperatureRecovery::GetRecoveryRatioDataName()
{
	static const FName DataName(TEXT("Data.Elemental.TemperatureRecoveryRatio"));
	return DataName;
}

FName UMAGameplayEffect_TemperatureRecovery::GetRecoveryAmountDataName()
{
	static const FName DataName(TEXT("Data.Elemental.TemperatureRecoveryAmount"));
	return DataName;
}
