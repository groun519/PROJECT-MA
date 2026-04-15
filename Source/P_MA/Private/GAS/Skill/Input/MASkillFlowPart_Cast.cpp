#include "GAS/Skill/Input/MASkillFlowPart_Cast.h"

#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/MASkillAbility.h"

void UMASkillFlowPart_Cast::StartFlow(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode)
{
	Super::StartFlow(SkillAbility, StartMode);
	ApplyInputBlockTag();
	StartCastDurationTimer();
}

void UMASkillFlowPart_Cast::StopFlow()
{
	StopCastDurationTimer();
	RemoveInputBlockTag();
	Super::StopFlow();
}

bool UMASkillFlowPart_Cast::ShouldAutoAdvanceOnMontageCompleted() const
{
	return CastDuration <= 0.f;
}

void UMASkillFlowPart_Cast::HandleCastDurationElapsed()
{
	StopCastDurationTimer();
	ActivateNextFlow();
}

void UMASkillFlowPart_Cast::StartCastDurationTimer()
{
	if (CastDuration <= 0.f) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CastDurationTimerHandle,
			this,
			&UMASkillFlowPart_Cast::HandleCastDurationElapsed,
			CastDuration,
			false);
	}
}

void UMASkillFlowPart_Cast::StopCastDurationTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CastDurationTimerHandle);
	}
}

void UMASkillFlowPart_Cast::ApplyInputBlockTag()
{
	if (bAppliedInputBlockTag) return;

	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAbilitySystemComponent* ASC = SkillAbility ? SkillAbility->GetAbilitySystemComponentFromActorInfo() : nullptr;
	if (!ASC) return;

	ASC->AddLooseGameplayTag(UMAAbilitySystemStatics::GetInputBlockTag());
	bAppliedInputBlockTag = true;
}

void UMASkillFlowPart_Cast::RemoveInputBlockTag()
{
	if (!bAppliedInputBlockTag) return;

	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAbilitySystemComponent* ASC = SkillAbility ? SkillAbility->GetAbilitySystemComponentFromActorInfo() : nullptr;
	if (!ASC) return;

	ASC->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetInputBlockTag());
	bAppliedInputBlockTag = false;
}

void UMASkillFlowPart_Cast::ActivateNextFlow()
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility) return;

	if (!SkillAbility->ActivatePreparedNextFlow())
	{
		SkillAbility->CompleteCurrentFlow();
	}
}
