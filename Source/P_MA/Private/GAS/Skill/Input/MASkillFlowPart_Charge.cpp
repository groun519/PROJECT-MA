#include "GAS/Skill/Input/MASkillFlowPart_Charge.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Skill/MASkillAbility.h"

UMASkillFlowPart_Charge::UMASkillFlowPart_Charge()
{
	FlowProgressSettings.bShowProgress = true;
	FlowProgressSettings.Label = FText::FromString(TEXT("Charge"));
}

void UMASkillFlowPart_Charge::OnTimedFlowStarted(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode)
{
	if (!SkillAbility) return;

	ChargeStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.f;

	if (StartMode == EMASkillFlowStartMode::Fresh)
	{
		SkillAbility->PrepareNextFlowMontage(ChargeDuration);
	}

	ArmInputRelease();
}

void UMASkillFlowPart_Charge::OnTimedFlowStopped()
{
	StopWaitingInputRelease();
	ChargeStartTime = -1.f;
}

void UMASkillFlowPart_Charge::HandleInputReleased(float /*TimeHeld*/)
{
	StopWaitingInputRelease();
	StopTimedFlow();
	CommitChargePayload();
	ChargeStartTime = -1.f;
	AdvanceOrCompleteOwnerFlow();
}

void UMASkillFlowPart_Charge::ArmInputRelease()
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility) return;

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(SkillAbility, false);
	InputReleaseTask->OnRelease.AddDynamic(this, &UMASkillFlowPart_Charge::HandleInputReleased);
	InputReleaseTask->ReadyForActivation();
}

void UMASkillFlowPart_Charge::StopWaitingInputRelease()
{
	if (!InputReleaseTask) return;

	InputReleaseTask->EndTask();
	InputReleaseTask = nullptr;
}

void UMASkillFlowPart_Charge::OnTimedFlowElapsed()
{
	StopWaitingInputRelease();
	CommitChargePayload();
	AdvanceOrCompleteOwnerFlow();
}

void UMASkillFlowPart_Charge::CommitChargePayload() const
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility) return;

	SkillAbility->GetPayloadStore().SetScalar(
		FGameplayTag::RequestGameplayTag(TEXT("Data.Skill.Payload.Scalar.ChargeRatio")),
		ResolveChargeRatio());
}

float UMASkillFlowPart_Charge::ResolveChargeRatio() const
{
	if (ChargeDuration <= 0.f) return 1.f;
	if (ChargeStartTime < 0.f) return 0.f;

	const UWorld* World = GetWorld();
	if (!World) return 0.f;

	return FMath::Clamp((World->GetTimeSeconds() - ChargeStartTime) / ChargeDuration, 0.f, 1.f);
}
