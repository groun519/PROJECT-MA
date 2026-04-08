#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControl.h"
#include "MASkillCrowdControl_State.generated.h"

UCLASS(Abstract, BlueprintType)
class P_MA_API UMASkillCrowdControlStateBase : public UMASkillCrowdControl
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	virtual FGameplayTag GetCrowdControlTag() const
		PURE_VIRTUAL(UMASkillCrowdControlStateBase::GetCrowdControlTag, return FGameplayTag(););

	virtual bool ResolvePolicy(FMASkillCrowdControlPolicy& OutPolicy) const override;
};

UCLASS(BlueprintType, DisplayName="CC Stun")
class P_MA_API UMASkillCrowdControlStun : public UMASkillCrowdControlStateBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
	virtual FMASkillCrowdControlGrantedStateRule GetGrantedStateRule() const override;
};

UCLASS(BlueprintType, DisplayName="CC Root")
class P_MA_API UMASkillCrowdControlRoot : public UMASkillCrowdControlStateBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
	virtual FMASkillCrowdControlGrantedStateRule GetGrantedStateRule() const override;
};
