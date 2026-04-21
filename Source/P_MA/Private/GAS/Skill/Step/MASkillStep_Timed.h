#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Step/MASkillStep.h"
#include "TimerManager.h"
#include "MASkillStep_Timed.generated.h"

USTRUCT(BlueprintType)
struct FMAStepProgressSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Progress")
	bool bShowProgress = false;

	UPROPERTY(EditDefaultsOnly, Category="Progress", meta=(EditCondition="bShowProgress"))
	FText Label;
};

UCLASS(Abstract, BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillStep_Timed : public UMASkillStep
{
	GENERATED_BODY()

public:
	virtual void StartStep(UMASkillAbility* SkillAbility, EMASkillStepStartMode StartMode) override;
	virtual void StopStep() override;
	virtual bool ShouldAutoAdvanceOnMontageCompleted() const override;
	virtual bool GetStepProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const override;

protected:
	virtual float GetStepDuration() const PURE_VIRTUAL(UMASkillStep_Timed::GetStepDuration, return 0.f;);
	virtual void OnTimedStepStarted(UMASkillAbility* SkillAbility, EMASkillStepStartMode StartMode) {}
	virtual void OnTimedStepStopped() {}
	virtual void OnTimedStepElapsed();
	void AdvanceOrCompleteOwnerStep();
	void StopTimedStep();
	void StartStepProgress(float Duration);
	void StopStepProgress();

	UPROPERTY(EditDefaultsOnly, Category="Progress")
	FMAStepProgressSettings StepProgressSettings;

	UPROPERTY(Transient)
	float StepProgressDuration = 0.f;

	UPROPERTY(Transient)
	float StepProgressEndTimeSeconds = 0.f;

private:
	UFUNCTION()
	void HandleTimedStepElapsed();

	void StartTimedStepTimer();
	void StopTimedStepTimer();

	FTimerHandle TimedStepTimerHandle;
};
