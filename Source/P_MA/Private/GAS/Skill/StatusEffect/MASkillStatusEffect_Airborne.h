#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"
#include "MASkillStatusEffect_Airborne.generated.h"

UCLASS(BlueprintType, DisplayName="SE Airborne")
class P_MA_API UMASkillStatusEffectAirborne : public UMASkillStatusEffect
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(ClampMin="0.0"))
	float Magnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(ClampMin="0.0"))
	float RiseTime = 0.f;

	virtual bool ResolvePolicy(FMASkillStatusEffectPolicy& OutPolicy) const override;
	virtual void ApplyCustomPayload(FGameplayEffectSpecHandle& SpecHandle) const override;
};
