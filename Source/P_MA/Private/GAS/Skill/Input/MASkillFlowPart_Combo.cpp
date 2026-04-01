#include "GAS/Skill/Input/MASkillFlowPart_Combo.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Animation/AnimInstance.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"

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

	template <typename TaskType>
	void EndTasksAndReset(TArray<TObjectPtr<TaskType>>& Tasks)
	{
		for (TaskType* Task : Tasks)
		{
			if (Task)
			{
				Task->EndTask();
			}
		}

		Tasks.Reset();
	}

	bool TryGetCurrentSkillSection(UMASkillAbility* SkillAbility, UAnimInstance*& OutAnimInstance, UAnimMontage*& OutSkillMontage, FName& OutCurrentSectionName)
	{
		if (!SkillAbility || !SkillAbility->GetSkillDefinition()) return false;

		OutAnimInstance = SkillAbility->GetOwnerAnimInstance();
		OutSkillMontage = SkillAbility->GetSkillDefinition()->GetSkillMontage();
		if (!OutAnimInstance || !OutSkillMontage) return false;

		OutCurrentSectionName = OutAnimInstance->Montage_GetCurrentSection(OutSkillMontage);
		return !OutCurrentSectionName.IsNone();
	}
}

void UMASkillFlowPart_Combo::StartFlow(UMASkillAbility* SkillAbility)
{
	Super::StartFlow(SkillAbility);

	ClearReservedState();
	RegisterComboEventWaits();
	ArmInputPress();
}

void UMASkillFlowPart_Combo::StopFlow()
{
	StopInputLoop();
	UnregisterComboEventWaits();
	ClearReservedState();

	Super::StopFlow();
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

void UMASkillFlowPart_Combo::HandleComboEventOpened(FGameplayEventData Payload)
{
	ClearCurrentSectionLink();
	ReservationState.ReservedNextSection = FindNextSectionByTag(Payload.EventTag);
}

void UMASkillFlowPart_Combo::HandleComboEventClosed(FGameplayEventData Payload)
{
	(void)Payload;
	ReservationState.ReservedNextSection = NAME_None;
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

void UMASkillFlowPart_Combo::RegisterComboEventWaits()
{
	if (!GetOwnerSkillAbility()) return;

	ComboWindowState.OpenEventTasks.Reset();
	for (const FMASkillComboInputEvent& Event : ComboEvents)
	{
		if (!Event.OpenTag.IsValid()) continue;

		UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(GetOwnerSkillAbility(), Event.OpenTag, nullptr, false, false);
		EventTask->EventReceived.AddDynamic(this, &UMASkillFlowPart_Combo::HandleComboEventOpened);
		EventTask->ReadyForActivation();
		ComboWindowState.OpenEventTasks.Add(EventTask);
	}

	if (CloseTag.IsValid())
	{
		ComboWindowState.CloseEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(GetOwnerSkillAbility(), CloseTag, nullptr, false, false);
		ComboWindowState.CloseEventTask->EventReceived.AddDynamic(this, &UMASkillFlowPart_Combo::HandleComboEventClosed);
		ComboWindowState.CloseEventTask->ReadyForActivation();
	}
}

void UMASkillFlowPart_Combo::UnregisterComboEventWaits()
{
	EndTasksAndReset(ComboWindowState.OpenEventTasks);
	EndTaskAndReset(ComboWindowState.CloseEventTask);
}

void UMASkillFlowPart_Combo::ClearCurrentSectionLink()
{
	UAnimInstance* OwnerAnimInstance = nullptr;
	UAnimMontage* SkillMontage = nullptr;
	FName CurrentSectionName = NAME_None;
	if (!TryGetCurrentSkillSection(GetOwnerSkillAbility(), OwnerAnimInstance, SkillMontage, CurrentSectionName))
	{
		return;
	}

	OwnerAnimInstance->Montage_SetNextSection(CurrentSectionName, NAME_None, SkillMontage);
}

void UMASkillFlowPart_Combo::CommitReservedNextSection()
{
	if (ReservationState.ReservedNextSection.IsNone()) return;

	UAnimInstance* OwnerAnimInstance = nullptr;
	UAnimMontage* SkillMontage = nullptr;
	FName CurrentSectionName = NAME_None;
	if (!TryGetCurrentSkillSection(GetOwnerSkillAbility(), OwnerAnimInstance, SkillMontage, CurrentSectionName))
	{
		return;
	}

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

FName UMASkillFlowPart_Combo::FindNextSectionByTag(const FGameplayTag& EventTag) const
{
	for (const FMASkillComboInputEvent& Event : ComboEvents)
	{
		if (Event.OpenTag == EventTag)
		{
			return Event.NextSectionName;
		}
	}

	return NAME_None;
}

void UMASkillFlowPart_Combo::ClearReservedState()
{
	ReservationState.ReservedNextSection = NAME_None;
}
