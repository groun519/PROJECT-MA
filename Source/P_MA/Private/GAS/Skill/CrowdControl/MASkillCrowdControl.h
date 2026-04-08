#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlTypes.h"
#include "MASkillCrowdControl.generated.h"

class UMASkillAbility;

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillCrowdControl : public UObject
{
	GENERATED_BODY()

public:
	bool BuildResolvedEffect(UMASkillAbility& SkillAbility, TArray<FResolvedCrowdControlEffect>& OutEffects) const;

protected:
	virtual bool ResolvePolicy(FMASkillCrowdControlPolicy& OutPolicy) const
		PURE_VIRTUAL(UMASkillCrowdControl::ResolvePolicy, return false;);

	virtual FMASkillCrowdControlGrantedStateRule GetGrantedStateRule() const { return FMASkillCrowdControlGrantedStateRule(); }
	virtual void ApplyCustomPayload(FGameplayEffectSpecHandle& SpecHandle) const {}

	static void AppendGrantedStateTags(const FMASkillCrowdControlGrantedStateRule& Rule, FGameplayTagContainer& GrantedTags);
	static FMASkillCrowdControlGrantedStateRule MakeFullBlockGrantedStateRule();
	static FMASkillCrowdControlGrantedStateRule MakeMoveOnlyGrantedStateRule();
};
