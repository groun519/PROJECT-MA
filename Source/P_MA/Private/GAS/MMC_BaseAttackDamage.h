#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_BaseAttackDamage.generated.h"

UCLASS()
class UMMC_BaseAttackDamage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()
	
public:
	UMMC_BaseAttackDamage();
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
private:
	FGameplayEffectAttributeCaptureDefinition DamageCaptureDef;
	FGameplayEffectAttributeCaptureDefinition ArmorCaptureDef;
	FGameplayEffectAttributeCaptureDefinition ArmorPenetrationCaptureDef;
	FGameplayEffectAttributeCaptureDefinition DamageVarianceCaptureDef;
	FGameplayEffectAttributeCaptureDefinition CriticalChanceCaptureDef;
	FGameplayEffectAttributeCaptureDefinition CriticalDamageCaptureDef;

	FGameplayTag BehaviorModifierTag;
	FGameplayTag UtilityModifierTag;
	FGameplayTag ElementalModifierTag;
};
