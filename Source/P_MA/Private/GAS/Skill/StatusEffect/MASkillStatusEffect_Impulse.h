#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"
#include "MASkillStatusEffect_Impulse.generated.h"

UCLASS(Abstract, BlueprintType)
class P_MA_API UMASkillStatusEffectImpulseBase : public UMASkillStatusEffect
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(ClampMin="0.0"))
	float Magnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="StatusEffect")
	EMASkillStatusEffectSourceType SourceType = EMASkillStatusEffectSourceType::Instigator;

	virtual FGameplayTag GetStatusEffectTag() const
		PURE_VIRTUAL(UMASkillStatusEffectImpulseBase::GetStatusEffectTag, return FGameplayTag(););

	virtual bool ResolvePolicy(FMASkillStatusEffectPolicy& OutPolicy) const override;
};

UCLASS(BlueprintType, DisplayName="SE Knockback")
class P_MA_API UMASkillStatusEffectKnockback : public UMASkillStatusEffectImpulseBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetStatusEffectTag() const override;
};

UCLASS(BlueprintType, DisplayName="SE Grab")
class P_MA_API UMASkillStatusEffectGrab : public UMASkillStatusEffectImpulseBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetStatusEffectTag() const override;
};

UCLASS(BlueprintType, DisplayName="SE Stagger")
class P_MA_API UMASkillStatusEffectStagger : public UMASkillStatusEffectImpulseBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetStatusEffectTag() const override;
};
