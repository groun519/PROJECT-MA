#include "GAS/Skill/Step/MASkillStep_Charge.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Skill/MASkillAbility.h"

UMASkillStep_Charge::UMASkillStep_Charge()
{
	StepProgressSettings.bShowProgress = true;
	StepProgressSettings.Label = FText::FromString(TEXT("Charge"));
}

void UMASkillStep_Charge::OnTimedStepStarted(UMASkillAbility* SkillAbility, EMASkillStepStartMode StartMode)
{
	if (!SkillAbility) return;

	ChargeStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.f;

	if (StartMode == EMASkillStepStartMode::Fresh)
	{
		SkillAbility->PrepareNextStepMontage(ChargeDuration);
	}

	ArmInputRelease();
}

void UMASkillStep_Charge::OnTimedStepStopped()
{
	StopWaitingInputRelease();
	ChargeStartTime = -1.f;
}

void UMASkillStep_Charge::HandleInputReleased(float /*TimeHeld*/)
{
	StopWaitingInputRelease();
	StopTimedStep();
	CommitChargePayload();
	ChargeStartTime = -1.f;
	AdvanceOrCompleteOwnerStep();
}

void UMASkillStep_Charge::ArmInputRelease()
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility) return;

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(SkillAbility, false);
	InputReleaseTask->OnRelease.AddDynamic(this, &UMASkillStep_Charge::HandleInputReleased);
	InputReleaseTask->ReadyForActivation();
}

void UMASkillStep_Charge::StopWaitingInputRelease()
{
	if (!InputReleaseTask) return;

	InputReleaseTask->EndTask();
	InputReleaseTask = nullptr;
}

void UMASkillStep_Charge::OnTimedStepElapsed()
{
	StopWaitingInputRelease();
	CommitChargePayload();
	AdvanceOrCompleteOwnerStep();
}

void UMASkillStep_Charge::CommitChargePayload() const
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility) return;

	SkillAbility->GetPayloadStore().SetScalar(
		FGameplayTag::RequestGameplayTag(TEXT("Data.Skill.Payload.Scalar.ChargeRatio")),
		ResolveChargeRatio());
}

float UMASkillStep_Charge::ResolveChargeRatio() const
{
	if (ChargeDuration <= 0.f) return 1.f;
	if (ChargeStartTime < 0.f) return 0.f;

	const UWorld* World = GetWorld();
	if (!World) return 0.f;

	return FMath::Clamp((World->GetTimeSeconds() - ChargeStartTime) / ChargeDuration, 0.f, 1.f);
}
