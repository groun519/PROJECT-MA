#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"
#include "TimerManager.h"
#include "MASkillFlowPart_Cast.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillFlowPart_Cast : public UMASkillFlowPart
{
	GENERATED_BODY()

public:
	virtual void StartFlow(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode) override;
	virtual void StopFlow() override;
	virtual bool ShouldAutoAdvanceOnMontageCompleted() const override;

private:
	UFUNCTION()
	void HandleCastDurationElapsed();

	void StartCastDurationTimer();
	void StopCastDurationTimer();
	void ApplyInputBlockTag();
	void RemoveInputBlockTag();
	void ActivateNextFlow();

	UPROPERTY(EditDefaultsOnly, Category="Cast", meta=(ClampMin="0.0"))
	float CastDuration = 0.f;

	UPROPERTY(Transient)
	bool bAppliedInputBlockTag = false;

	FTimerHandle CastDurationTimerHandle;
};
