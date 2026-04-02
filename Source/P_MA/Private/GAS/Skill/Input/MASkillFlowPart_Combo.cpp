#include "GAS/Skill/Input/MASkillFlowPart_Combo.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

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

void UMASkillFlowPart_Combo::StartFlow(UMASkillAbility* SkillAbility, FSkillRuntimeContext* InRuntimeContext)
{
	Super::StartFlow(SkillAbility, InRuntimeContext);

	ClearReservedState();
	ArmInputPress();
}

void UMASkillFlowPart_Combo::StopFlow()
{
	StopInputLoop();
	ClearReservedState();

	Super::StopFlow();
}

void UMASkillFlowPart_Combo::CollectRequiredEventTags(TSet<FGameplayTag>& OutTags) const
{
	for (const FMASkillComboInputEvent& Event : ComboEvents)
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

void UMASkillFlowPart_Combo::HandleRuntimeEvent(const FGameplayEventData& Payload)
{
	for (const FMASkillComboInputEvent& Event : ComboEvents)
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

void UMASkillFlowPart_Combo::HandleInputPressed(float TimeWaited)
{
	(void)TimeWaited;

	InputLoopState.InputPressTask = nullptr;
	CommitReservedNextSection();
	StartHoldLoop();
	ArmInputRelease();
}

void UMASkillFlowPart_Combo::HandleInputReleased(float TimeHeld)
{
	(void)TimeHeld;

	InputLoopState.InputReleaseTask = nullptr;
	StopHoldLoop();
	ArmInputPress();
}

void UMASkillFlowPart_Combo::HandleHoldTick()
{
	CommitReservedNextSection();
}

void UMASkillFlowPart_Combo::ArmInputPress()
{
	if (!GetOwnerSkillAbility()) return;

	InputLoopState.InputPressTask = UAbilityTask_WaitInputPress::WaitInputPress(GetOwnerSkillAbility(), true);
	InputLoopState.InputPressTask->OnPress.AddDynamic(this, &UMASkillFlowPart_Combo::HandleInputPressed);
	InputLoopState.InputPressTask->ReadyForActivation();
}

void UMASkillFlowPart_Combo::ArmInputRelease()
{
	if (!GetOwnerSkillAbility()) return;

	InputLoopState.InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(GetOwnerSkillAbility(), false);
	InputLoopState.InputReleaseTask->OnRelease.AddDynamic(this, &UMASkillFlowPart_Combo::HandleInputReleased);
	InputLoopState.InputReleaseTask->ReadyForActivation();
}

void UMASkillFlowPart_Combo::StartHoldLoop()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			InputLoopState.HoldTimerHandle,
			this,
			&UMASkillFlowPart_Combo::HandleHoldTick,
			HoldInterval,
			true,
			HoldInterval);
	}
}

void UMASkillFlowPart_Combo::StopInputLoop()
{
	StopHoldLoop();
	EndTaskAndReset(InputLoopState.InputPressTask);
	EndTaskAndReset(InputLoopState.InputReleaseTask);
}

void UMASkillFlowPart_Combo::ClearCurrentSectionLink()
{
	FSkillRuntimeContext* RuntimeContextPtr = GetRuntimeContext();
	UAnimInstance* OwnerAnimInstance = nullptr;
	UAnimMontage* SkillMontage = nullptr;
	FName CurrentSectionName = NAME_None;
	if (!RuntimeContextPtr || !RuntimeContextPtr->TryGetCurrentSkillSection(OwnerAnimInstance, SkillMontage, CurrentSectionName)) return;

	OwnerAnimInstance->Montage_SetNextSection(CurrentSectionName, NAME_None, SkillMontage);
}

void UMASkillFlowPart_Combo::CommitReservedNextSection()
{
	if (ReservationState.ReservedNextSection.IsNone()) return;

	FSkillRuntimeContext* RuntimeContextPtr = GetRuntimeContext();
	UAnimInstance* OwnerAnimInstance = nullptr;
	UAnimMontage* SkillMontage = nullptr;
	FName CurrentSectionName = NAME_None;
	if (!RuntimeContextPtr || !RuntimeContextPtr->TryGetCurrentSkillSection(OwnerAnimInstance, SkillMontage, CurrentSectionName)) return;

	if (CurrentSectionName == ReservationState.ReservedNextSection) return;

	OwnerAnimInstance->Montage_SetNextSection(CurrentSectionName, ReservationState.ReservedNextSection, SkillMontage);
}

void UMASkillFlowPart_Combo::StopHoldLoop()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InputLoopState.HoldTimerHandle);
	}
}

void UMASkillFlowPart_Combo::ClearReservedState()
{
	ReservationState.ReservedNextSection = NAME_None;
}
