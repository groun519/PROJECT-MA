#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Step/MASkillStep_Timed.h"
#include "MASkillStep_Cast.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillStep_Cast : public UMASkillStep_Timed
{
	GENERATED_BODY()

public:
	UMASkillStep_Cast();

private:
	virtual float GetStepDuration() const override { return CastDuration; }
	virtual void OnTimedStepStarted(UMASkillAbility*, EMASkillStepStartMode) override;
	virtual void OnTimedStepStopped() override;
	void ApplyInputBlockTag();
	void RemoveInputBlockTag();

	UPROPERTY(EditDefaultsOnly, Category="Cast", meta=(ClampMin="0.0"))
	float CastDuration = 0.f;

	UPROPERTY(Transient)
	bool bAppliedInputBlockTag = false;
};
