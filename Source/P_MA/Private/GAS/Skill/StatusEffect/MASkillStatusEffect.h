#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffectTypes.h"
#include "MASkillStatusEffect.generated.h"

class UMASkillAbility;
class UGameplayEffect;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillStatusEffect : public UObject
{
	GENERATED_BODY()

public:
	virtual bool BuildResolvedEffect(UMASkillAbility& SkillAbility, TArray<FResolvedStatusEffect>& OutEffects) const;

protected:
	virtual bool ResolvePolicy(FMASkillStatusEffectPolicy& OutPolicy) const
		PURE_VIRTUAL(UMASkillStatusEffect::ResolvePolicy, return false;);

	virtual FMASkillStatusEffectGrantedStateRule GetGrantedStateRule() const { return FMASkillStatusEffectGrantedStateRule(); }
	virtual void ApplyCustomPayload(FGameplayEffectSpecHandle& SpecHandle) const {}

	static FGameplayEffectSpecHandle MakeGameplayEffectSpec(UMASkillAbility& SkillAbility, const UGameplayEffect* EffectDefinition, float Level);
	static void AppendGrantedStateTags(const FMASkillStatusEffectGrantedStateRule& Rule, FGameplayTagContainer& GrantedTags);
};
