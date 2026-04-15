#include "GAS/Skill/Input/MASkillFlowPart_Delay.h"

#include "GAS/Skill/MASkillAbility.h"

void UMASkillFlowPart_Delay::StartFlow(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode)
{
	Super::StartFlow(SkillAbility, StartMode);
	StartDelayTimer();
}

void UMASkillFlowPart_Delay::StopFlow()
{
	StopDelayTimer();
	Super::StopFlow();
}

bool UMASkillFlowPart_Delay::ShouldAutoAdvanceOnMontageCompleted() const
{
	return DelayDuration <= 0.f;
}

void UMASkillFlowPart_Delay::HandleDelayElapsed()
{
	StopDelayTimer();
	ActivateNextFlow();
}

void UMASkillFlowPart_Delay::StartDelayTimer()
{
	if (DelayDuration <= 0.f) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			DelayTimerHandle,
			this,
			&UMASkillFlowPart_Delay::HandleDelayElapsed,
			DelayDuration,
			false);
	}
}

void UMASkillFlowPart_Delay::StopDelayTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DelayTimerHandle);
	}
}

void UMASkillFlowPart_Delay::ActivateNextFlow()
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility) return;

	if (!SkillAbility->ActivatePreparedNextFlow())
	{
		SkillAbility->CompleteCurrentFlow();
	}
}
