#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControl.h"
#include "MASkillCrowdControl_Airborne.generated.h"

UCLASS(BlueprintType, DisplayName="CC Airborne")
class P_MA_API UMASkillCrowdControlAirborne : public UMASkillCrowdControl
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Magnitude = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float Duration = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="CrowdControl", meta=(ClampMin="0.0"))
	float RiseTime = 0.f;

	virtual bool ResolvePolicy(FMASkillCrowdControlPolicy& OutPolicy) const override;
	virtual void ApplyCustomPayload(FGameplayEffectSpecHandle& SpecHandle) const override;
};
