#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_DamageByAttribute.generated.h"

enum class EMADamageAttributeSide : uint8;

UCLASS()
class P_MA_API UExecCalc_DamageByAttribute : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_DamageByAttribute();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

private:
	struct FAttributeCaptureDefinitions
	{
		FGameplayEffectAttributeCaptureDefinition Source;
		FGameplayEffectAttributeCaptureDefinition Target;
		FName SourceCoefficientName;
		FName TargetCoefficientName;
	};

	const FGameplayEffectAttributeCaptureDefinition* FindCaptureDefinition(
		EMADamageAttributeSide Side,
		const FGameplayAttribute& Attribute) const;

	TMap<FGameplayAttribute, FAttributeCaptureDefinitions> AttributeCaptureDefinitions;

	FGameplayTag BehaviorModifierTag;
	FGameplayTag DamageVarianceTag;
};
