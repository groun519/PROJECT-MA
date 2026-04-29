#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect.h"
#include "MASkillStatusEffect_State.generated.h"

UCLASS(Abstract, BlueprintType)
class P_MA_API UMASkillStatusEffectStateBase : public UMASkillStatusEffect
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="StatusEffect", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	virtual FGameplayTag GetStatusEffectTag() const
		PURE_VIRTUAL(UMASkillStatusEffectStateBase::GetStatusEffectTag, return FGameplayTag(););

	virtual bool ResolvePolicy(FMASkillStatusEffectPolicy& OutPolicy) const override;
};

UCLASS(BlueprintType, DisplayName="SE Stun")
class P_MA_API UMASkillStatusEffectStun : public UMASkillStatusEffectStateBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetStatusEffectTag() const override;
	virtual FMASkillStatusEffectGrantedStateRule GetGrantedStateRule() const override;
};

UCLASS(BlueprintType, DisplayName="SE Root")
class P_MA_API UMASkillStatusEffectRoot : public UMASkillStatusEffectStateBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetStatusEffectTag() const override;
	virtual FMASkillStatusEffectGrantedStateRule GetGrantedStateRule() const override;
};
