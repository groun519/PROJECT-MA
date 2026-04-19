#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Input/MASkillFlowPart_Timed.h"
#include "MASkillFlowPart_Cast.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillFlowPart_Cast : public UMASkillFlowPart_Timed
{
	GENERATED_BODY()

public:
	UMASkillFlowPart_Cast();

private:
	virtual float GetFlowDuration() const override { return CastDuration; }
	virtual void OnTimedFlowStarted(UMASkillAbility*, EMASkillFlowStartMode) override;
	virtual void OnTimedFlowStopped() override;
	void ApplyInputBlockTag();
	void RemoveInputBlockTag();

	UPROPERTY(EditDefaultsOnly, Category="Cast", meta=(ClampMin="0.0"))
	float CastDuration = 0.f;

	UPROPERTY(Transient)
	bool bAppliedInputBlockTag = false;
};
