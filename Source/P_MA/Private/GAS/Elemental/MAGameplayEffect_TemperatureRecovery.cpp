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
	const UMAGameplayEffect_TemperatureRecovery* RecoveryEffect = CastChecked<UMAGameplayEffect_TemperatureRecovery>(Spec.Def);

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Temperature = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(TargetTemperatureDef, EvalParams, Temperature);
	if (FMath::IsNearlyZero(Temperature)) return;

	const float AbsTemperature = FMath::Abs(Temperature);
	const float Delta = AbsTemperature <= RecoveryEffect->GetSnapThreshold()
		? -Temperature
		: -Temperature * RecoveryEffect->GetRecoveryRatioPerTick();
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
