#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"
#include "TimerManager.h"
#include "MASkillFlowPart_Timed.generated.h"

USTRUCT(BlueprintType)
struct FMAFlowProgressSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Progress")
	bool bShowProgress = false;

	UPROPERTY(EditDefaultsOnly, Category="Progress", meta=(EditCondition="bShowProgress"))
	FText Label;
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillFlowPart_Timed : public UMASkillFlowPart
{
	GENERATED_BODY()

public:
	virtual void StartFlow(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode) override;
	virtual void StopFlow() override;
	virtual bool ShouldAutoAdvanceOnMontageCompleted() const override;
	bool GetFlowProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const;

protected:
	virtual float GetFlowDuration() const PURE_VIRTUAL(UMASkillFlowPart_Timed::GetFlowDuration, return 0.f;);
	virtual void OnTimedFlowStarted(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode) {}
	virtual void OnTimedFlowStopped() {}
	virtual void OnTimedFlowElapsed();
	void AdvanceOrCompleteOwnerFlow();
	void StopTimedFlow();
	void StartFlowProgress(float Duration);
	void StopFlowProgress();

	UPROPERTY(EditDefaultsOnly, Category="Progress")
	FMAFlowProgressSettings FlowProgressSettings;

	UPROPERTY(Transient)
	float FlowProgressDuration = 0.f;

	UPROPERTY(Transient)
	float FlowProgressEndTimeSeconds = 0.f;

private:
	UFUNCTION()
	void HandleTimedFlowElapsed();

	void StartTimedFlowTimer();
	void StopTimedFlowTimer();

	FTimerHandle TimedFlowTimerHandle;
};
