#include "GAS/Skill/Input/MASkillFlowPart_Timed.h"

#include "GAS/Skill/MASkillAbility.h"

void UMASkillFlowPart_Timed::StartFlow(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode)
{
	Super::StartFlow(SkillAbility, StartMode);
	StartFlowProgress(GetFlowDuration());
	StartTimedFlowTimer();
	OnTimedFlowStarted(SkillAbility, StartMode);
}

void UMASkillFlowPart_Timed::StopFlow()
{
	StopTimedFlow();
	OnTimedFlowStopped();
	Super::StopFlow();
}

bool UMASkillFlowPart_Timed::ShouldAutoAdvanceOnMontageCompleted() const
{
	return GetFlowDuration() <= 0.f;
}

bool UMASkillFlowPart_Timed::GetFlowProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const
{
	if (!FlowProgressSettings.bShowProgress || FlowProgressDuration <= 0.f)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	const float RemainingDuration = FMath::Max(FlowProgressEndTimeSeconds - World->GetTimeSeconds(), 0.f);
	if (RemainingDuration <= 0.f)
	{
		return false;
	}

	OutLabel = FlowProgressSettings.Label;
	OutDuration = FlowProgressDuration;
	OutRemainingDuration = RemainingDuration;
	return true;
}

void UMASkillFlowPart_Timed::OnTimedFlowElapsed()
{
	AdvanceOrCompleteOwnerFlow();
}

void UMASkillFlowPart_Timed::AdvanceOrCompleteOwnerFlow()
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility)
	{
		return;
	}

	if (!SkillAbility->ActivatePreparedNextFlow())
	{
		SkillAbility->CompleteCurrentFlow();
	}
}

void UMASkillFlowPart_Timed::StopTimedFlow()
{
	StopFlowProgress();
	StopTimedFlowTimer();
}

void UMASkillFlowPart_Timed::StartFlowProgress(float Duration)
{
	if (!FlowProgressSettings.bShowProgress || Duration <= 0.f)
	{
		StopFlowProgress();
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		StopFlowProgress();
		return;
	}

	FlowProgressDuration = Duration;
	FlowProgressEndTimeSeconds = World->GetTimeSeconds() + Duration;
}

void UMASkillFlowPart_Timed::StopFlowProgress()
{
	FlowProgressDuration = 0.f;
	FlowProgressEndTimeSeconds = 0.f;
}

void UMASkillFlowPart_Timed::HandleTimedFlowElapsed()
{
	StopTimedFlowTimer();
	OnTimedFlowElapsed();
}

void UMASkillFlowPart_Timed::StartTimedFlowTimer()
{
	const float Duration = GetFlowDuration();
	if (Duration <= 0.f)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TimedFlowTimerHandle,
			this,
			&UMASkillFlowPart_Timed::HandleTimedFlowElapsed,
			Duration,
			false);
	}
}

void UMASkillFlowPart_Timed::StopTimedFlowTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimedFlowTimerHandle);
	}
}
