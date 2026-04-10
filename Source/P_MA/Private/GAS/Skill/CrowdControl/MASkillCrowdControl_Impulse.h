#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControl.h"
#include "MASkillCrowdControl_Impulse.generated.h"

UCLASS(Abstract, BlueprintType)
class P_MA_API UMASkillCrowdControlImpulseBase : public UMASkillCrowdControl
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Magnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl")
	EMASkillCrowdControlSourceType SourceType = EMASkillCrowdControlSourceType::Instigator;

	virtual FGameplayTag GetCrowdControlTag() const
		PURE_VIRTUAL(UMASkillCrowdControlImpulseBase::GetCrowdControlTag, return FGameplayTag(););

	virtual bool ResolvePolicy(FMASkillCrowdControlPolicy& OutPolicy) const override;
};

UCLASS(BlueprintType, DisplayName="CC Knockback")
class P_MA_API UMASkillCrowdControlKnockback : public UMASkillCrowdControlImpulseBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
};

UCLASS(BlueprintType, DisplayName="CC Grab")
class P_MA_API UMASkillCrowdControlGrab : public UMASkillCrowdControlImpulseBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
};

UCLASS(BlueprintType, DisplayName="CC Stagger")
class P_MA_API UMASkillCrowdControlStagger : public UMASkillCrowdControlImpulseBase
{
	GENERATED_BODY()

protected:
	virtual FGameplayTag GetCrowdControlTag() const override;
};
