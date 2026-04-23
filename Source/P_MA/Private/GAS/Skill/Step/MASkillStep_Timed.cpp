#include "GAS/Skill/Step/MASkillStep_Timed.h"

#include "GAS/Skill/MASkillAbility.h"

void UMASkillStep_Timed::StartStep(UMASkillAbility* SkillAbility, EMASkillStepStartMode StartMode)
{
	Super::StartStep(SkillAbility, StartMode);
	StartStepProgress(GetStepDuration());
	StartTimedStepTimer();
	OnTimedStepStarted(SkillAbility, StartMode);
}

void UMASkillStep_Timed::StopStep()
{
	StopTimedStep();
	OnTimedStepStopped();
	Super::StopStep();
}

bool UMASkillStep_Timed::ShouldAutoAdvanceOnMontageCompleted() const
{
	return GetStepDuration() <= 0.f;
}

bool UMASkillStep_Timed::GetStepProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const
{
	if (!StepProgressSettings.bShowProgress || StepProgressDuration <= 0.f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float RemainingDuration = FMath::Max(StepProgressEndTimeSeconds - World->GetTimeSeconds(), 0.f);
	if (RemainingDuration <= 0.f)
	{
		return false;
	}

	OutLabel = StepProgressSettings.Label;
	OutDuration = StepProgressDuration;
	OutRemainingDuration = RemainingDuration;
	return true;
}

void UMASkillStep_Timed::OnTimedStepElapsed()
{
	AdvanceOrCompleteOwnerStep();
}

void UMASkillStep_Timed::AdvanceOrCompleteOwnerStep()
{
	if (!ActivatePreparedNextStepPreview())
	{
	RequestAdvanceOrEnd();
	}
}

void UMASkillStep_Timed::StopTimedStep()
{
	StopStepProgress();
	StopTimedStepTimer();
}

void UMASkillStep_Timed::StartStepProgress(float Duration)
{
	if (!StepProgressSettings.bShowProgress || Duration <= 0.f)
	{
		StopStepProgress();
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		StopStepProgress();
		return;
	}

	StepProgressDuration = Duration;
	StepProgressEndTimeSeconds = World->GetTimeSeconds() + Duration;
}

void UMASkillStep_Timed::StopStepProgress()
{
	StepProgressDuration = 0.f;
	StepProgressEndTimeSeconds = 0.f;
}

void UMASkillStep_Timed::HandleTimedStepElapsed()
{
	StopTimedStepTimer();
	OnTimedStepElapsed();
}

void UMASkillStep_Timed::StartTimedStepTimer()
{
	const float Duration = GetStepDuration();
	if (Duration <= 0.f)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TimedStepTimerHandle,
			this,
			&UMASkillStep_Timed::HandleTimedStepElapsed,
			Duration,
			false);
	}
}

void UMASkillStep_Timed::StopTimedStepTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimedStepTimerHandle);
	}
}
