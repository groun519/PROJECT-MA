#include "GAS/Skill/Input/MASkillFlowPart_Charge.h"

#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Skill/MASkillAbility.h"

void UMASkillFlowPart_Charge::StartFlow(UMASkillAbility* SkillAbility, EMASkillFlowStartMode StartMode)
{
	Super::StartFlow(SkillAbility, StartMode);
	if (!SkillAbility) return;

	ChargeStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.f;

	if (StartMode == EMASkillFlowStartMode::Fresh)
	{
		SkillAbility->PrepareNextFlowMontage(ChargeDuration);
	}

	ArmInputRelease();
	StartChargeDurationTimer();
}

void UMASkillFlowPart_Charge::StopFlow()
{
	StopWaitingInputRelease();
	StopChargeDurationTimer();
	ChargeStartTime = -1.f;

	Super::StopFlow();
}

void UMASkillFlowPart_Charge::HandleInputReleased(float /*TimeHeld*/)
{
	StopWaitingInputRelease();
	StopChargeDurationTimer();
	CommitChargePayload();

	ActivateNextFlow();
}

void UMASkillFlowPart_Charge::HandleChargeDurationElapsed()
{
	StopWaitingInputRelease();
	StopChargeDurationTimer();
	CommitChargePayload();
	ActivateNextFlow();
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

void UMASkillFlowPart_Charge::StartChargeDurationTimer()
{
	if (ChargeDuration <= 0.f) return;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			ChargeDurationTimerHandle,
			this,
			&UMASkillFlowPart_Charge::HandleChargeDurationElapsed,
			ChargeDuration,
			false);
	}
}

void UMASkillFlowPart_Charge::StopChargeDurationTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ChargeDurationTimerHandle);
	}
}

void UMASkillFlowPart_Charge::CommitChargePayload() const
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility) return;

	SkillAbility->GetPayloadStore().SetScalar(
		FGameplayTag::RequestGameplayTag(TEXT("Data.Skill.Payload.Scalar.ChargeRatio")),
		ResolveChargeRatio());
}

void UMASkillFlowPart_Charge::ActivateNextFlow()
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility) return;

	if (!SkillAbility->ActivatePreparedNextFlow())
	{
		SkillAbility->CompleteCurrentFlow();
	}
}

float UMASkillFlowPart_Charge::ResolveChargeRatio() const
{
	if (ChargeDuration <= 0.f) return 1.f;
	if (ChargeStartTime < 0.f) return 0.f;

	const UWorld* World = GetWorld();
	if (!World) return 0.f;

	return FMath::Clamp((World->GetTimeSeconds() - ChargeStartTime) / ChargeDuration, 0.f, 1.f);
}
