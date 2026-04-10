#include "GAS/Skill/Input/MASkillFlowPart_AttackSequence.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Animation/AnimInstance.h"
#include "GAS/Skill/MASkillAbility.h"

namespace
{
	template <typename TaskType>
	void EndTaskAndReset(TObjectPtr<TaskType>& Task)
	{
		if (Task)
		{
			Task->EndTask();
			Task = nullptr;
		}
	}
}

void UMASkillFlowPart_AttackSequence::StartFlow(UMASkillAbility* SkillAbility)
{
	Super::StartFlow(SkillAbility);

	ClearReservedState();
	ArmInputPress();
}

void UMASkillFlowPart_AttackSequence::StopFlow()
{
	StopInputLoop();
	ClearReservedState();

	Super::StopFlow();
}

void UMASkillFlowPart_AttackSequence::CollectRequiredEventTags(TSet<FGameplayTag>& OutTags) const
{
	for (const FMASkillAttackSequenceEvent& Event : AttackSequenceEvents)
	{
		if (Event.OpenTag.IsValid())
		{
			OutTags.Add(Event.OpenTag);
		}
	}

	if (CloseTag.IsValid())
	{
		OutTags.Add(CloseTag);
	}
}

void UMASkillFlowPart_AttackSequence::HandleRuntimeEvent(const FGameplayEventData& Payload)
{
	for (const FMASkillAttackSequenceEvent& Event : AttackSequenceEvents)
	{
		if (Event.OpenTag != Payload.EventTag) continue;
		ClearCurrentSectionLink();
		ReservationState.ReservedNextSection = Event.NextSectionName;
		return;
	}

	if (CloseTag == Payload.EventTag)
	{
		ReservationState.ReservedNextSection = NAME_None;
	}
}

void UMASkillFlowPart_AttackSequence::HandleInputPressed(float TimeWaited)
{
	(void)TimeWaited;

	InputLoopState.InputPressTask = nullptr;
	CommitReservedNextSection();
	StartHoldLoop();
	ArmInputRelease();
}

void UMASkillFlowPart_AttackSequence::HandleInputReleased(float TimeHeld)
{
	(void)TimeHeld;

	InputLoopState.InputReleaseTask = nullptr;
	StopHoldLoop();
	ArmInputPress();
}

void UMASkillFlowPart_AttackSequence::HandleHoldTick()
{
	CommitReservedNextSection();
}

void UMASkillFlowPart_AttackSequence::ArmInputPress()
{
	if (!GetOwnerSkillAbility()) return;

	InputLoopState.InputPressTask = UAbilityTask_WaitInputPress::WaitInputPress(GetOwnerSkillAbility(), true);
	InputLoopState.InputPressTask->OnPress.AddDynamic(this, &UMASkillFlowPart_AttackSequence::HandleInputPressed);
	InputLoopState.InputPressTask->ReadyForActivation();
}

void UMASkillFlowPart_AttackSequence::ArmInputRelease()
{
	if (!GetOwnerSkillAbility()) return;

	InputLoopState.InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(GetOwnerSkillAbility(), false);
	InputLoopState.InputReleaseTask->OnRelease.AddDynamic(this, &UMASkillFlowPart_AttackSequence::HandleInputReleased);
	InputLoopState.InputReleaseTask->ReadyForActivation();
}

void UMASkillFlowPart_AttackSequence::StartHoldLoop()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			InputLoopState.HoldTimerHandle,
			this,
			&UMASkillFlowPart_AttackSequence::HandleHoldTick,
			HoldInterval,
			true,
			HoldInterval);
	}
}

void UMASkillFlowPart_AttackSequence::StopInputLoop()
{
	StopHoldLoop();
	EndTaskAndReset(InputLoopState.InputPressTask);
	EndTaskAndReset(InputLoopState.InputReleaseTask);
}

bool UMASkillFlowPart_AttackSequence::TryGetCurrentSectionContext(UAnimInstance*& OutAnimInstance, UAnimMontage*& OutSkillMontage, FName& OutCurrentSectionName) const
{
	const UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	if (!SkillAbility) return false;

	OutAnimInstance = SkillAbility->GetOwnerAnimInstance();
	OutSkillMontage = ResolveFlowMontage();
	if (!OutAnimInstance || !OutSkillMontage) return false;

	OutCurrentSectionName = OutAnimInstance->Montage_GetCurrentSection(OutSkillMontage);
	return !OutCurrentSectionName.IsNone();
}

void UMASkillFlowPart_AttackSequence::ClearCurrentSectionLink()
{
	UAnimInstance* OwnerAnimInstance = nullptr;
	UAnimMontage* SkillMontage = nullptr;
	FName CurrentSectionName = NAME_None;
	if (!TryGetCurrentSectionContext(OwnerAnimInstance, SkillMontage, CurrentSectionName)) return;

	OwnerAnimInstance->Montage_SetNextSection(CurrentSectionName, NAME_None, SkillMontage);
}

void UMASkillFlowPart_AttackSequence::CommitReservedNextSection()
{
	if (ReservationState.ReservedNextSection.IsNone()) return;

	UAnimInstance* OwnerAnimInstance = nullptr;
	UAnimMontage* SkillMontage = nullptr;
	FName CurrentSectionName = NAME_None;
	if (!TryGetCurrentSectionContext(OwnerAnimInstance, SkillMontage, CurrentSectionName)) return;

	if (CurrentSectionName == ReservationState.ReservedNextSection) return;

	OwnerAnimInstance->Montage_SetNextSection(CurrentSectionName, ReservationState.ReservedNextSection, SkillMontage);
}

void UMASkillFlowPart_AttackSequence::StopHoldLoop()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InputLoopState.HoldTimerHandle);
	}
}

void UMASkillFlowPart_AttackSequence::ClearReservedState()
{
	ReservationState.ReservedNextSection = NAME_None;
}
