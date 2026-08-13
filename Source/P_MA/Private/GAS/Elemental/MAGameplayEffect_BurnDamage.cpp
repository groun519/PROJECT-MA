#include "GAS/Elemental/MAGameplayEffect_BurnDamage.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAGameplayAbilityTypes.h"

UExecCalc_BurnDamage::UExecCalc_BurnDamage()
{
	TargetTemperatureDef.AttributeToCapture = UMAAttributeSet::GetTemperatureAttribute();
	TargetTemperatureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	TargetTemperatureDef.bSnapshot = false;
	RelevantAttributesToCapture.Add(TargetTemperatureDef);
}

void UExecCalc_BurnDamage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	const float MaxBurnDamage = Spec.GetSetByCallerMagnitude(UMAGameplayEffect_BurnDamage::GetMaxBurnDamageDataName(), false, 0.f);
	if (MaxBurnDamage <= 0.f) return;

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	float Temperature = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(TargetTemperatureDef, EvalParams, Temperature);
	if (Temperature <= 0.f) return;

	const float BurnDamage = FMath::Lerp(
		1.f,
		FMath::Max(MaxBurnDamage, 1.f),
		FMath::Clamp(Temperature / 100.f, 0.f, 1.f));

	if (FMAGameplayEffectContext* MAContext = static_cast<FMAGameplayEffectContext*>(Spec.GetContext().Get()))
	{
		MAContext->SetDamageTypeTag(UMAAbilitySystemStatics::GetDefaultDamageTypeTag());
		MAContext->SetDisplayMagnitude(BurnDamage);
	}

	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
		UMAAttributeSet::GetHealthAttribute(),
		EGameplayModOp::Additive,
		-BurnDamage));
}

UMAGameplayEffect_BurnDamage::UMAGameplayEffect_BurnDamage()
{
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	bExecutePeriodicEffectOnApplication = false;
	StackingType = EGameplayEffectStackingType::AggregateByTarget;
	StackLimitCount = 1;
	StackPeriodResetPolicy = EGameplayEffectStackingPeriodPolicy::NeverReset;

	FGameplayEffectExecutionDefinition& ExecutionDefinition = Executions.AddDefaulted_GetRef();
	ExecutionDefinition.CalculationClass = UExecCalc_BurnDamage::StaticClass();
}

FName UMAGameplayEffect_BurnDamage::GetMaxBurnDamageDataName()
{
	static const FName DataName(TEXT("Data.Elemental.MaxBurnDamage"));
	return DataName;
}
