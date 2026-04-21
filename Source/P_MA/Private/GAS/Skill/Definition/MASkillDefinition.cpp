#include "GAS/Skill/Definition/MASkillDefinition.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Event/MASkillEventSource.h"
#include "GAS/Skill/MASkillAbility.h"

namespace
{
int32 ResolveNextMontageStepIndex(const TArray<TObjectPtr<UMASkillStep>>& RuntimeSkillSteps, int32 CurrentIndex)
{
	for (int32 StepIndex = CurrentIndex + 1; StepIndex < RuntimeSkillSteps.Num(); ++StepIndex)
	{
		const UMASkillStep* RuntimeStep = RuntimeSkillSteps[StepIndex];
		if (RuntimeStep && RuntimeStep->ResolveStepMontage())
		{
			return StepIndex;
		}
	}

	return INDEX_NONE;
}
}

void UMASkillDefinition::ActivateSkill(UMASkillAbility* SkillAbility)
{
	OwnerSkillAbility = SkillAbility;
	InitializeRuntimeState(SkillAbility);

	CurrentStepIndex = SkillSteps.IsEmpty() ? INDEX_NONE : 0;
	CurrentStepStartMode = EMASkillStepStartMode::Fresh;

	for (UMASkillEventSource* RuntimeEventSource : EventSources)
	{
		if (!RuntimeEventSource) continue;
		RuntimeEventSource->StartSource(OwnerSkillAbility);
	}

	EnterCurrentStep();
	RebindEventTasks();
}

void UMASkillDefinition::DeactivateSkill()
{
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->StopActiveStep();
	}

	ClearPreparedStepPreviews();

	for (UMASkillEventSource* RuntimeEventSource : EventSources)
	{
		if (!RuntimeEventSource) continue;
		RuntimeEventSource->StopSource();
	}

	for (UAbilityTask_WaitGameplayEvent* Task : EventTasks)
	{
		if (Task) Task->EndTask();
	}

	EventTasks.Reset();

	CurrentStepIndex = INDEX_NONE;
	CurrentStepStartMode = EMASkillStepStartMode::Fresh;
	OwnerSkillAbility = nullptr;
}

void UMASkillDefinition::HandleSkillTagEvent(const FGameplayTag& EventTag, FSkillRuntimeContext& RuntimeContext, FMASkillPayloadStore& PayloadStore)
{
	if (!EventTag.IsValid()) return;

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	HandleSkillGameplayEvent(Payload, RuntimeContext, PayloadStore);
}

void UMASkillDefinition::HandleSkillGameplayEvent(FGameplayEventData Payload, FSkillRuntimeContext& RuntimeContext, FMASkillPayloadStore& PayloadStore)
{
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->HandleRuntimeEvent(Payload);
	}

	if (const TArray<TObjectPtr<UMASkillAction>>* ResolvedActions = ResolvedActionsByEvent.Find(Payload.EventTag))
	{
		if (!OwnerSkillAbility) return;

		for (UMASkillAction* Action : *ResolvedActions)
		{
			if (!Action) continue;
			Action->Execute(*OwnerSkillAbility, RuntimeContext, PayloadStore, Payload);
		}
	}
}

void UMASkillDefinition::ApplyDesiredMontagePlayRate(float DesiredMontagePlayRate) const
{
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->ApplyDesiredMontagePlayRate(DesiredMontagePlayRate);
	}
}

bool UMASkillDefinition::GetSkillProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const
{
	if (const UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		return CurrentStep->GetStepProgressInfo(OutLabel, OutDuration, OutRemainingDuration);
	}

	return false;
}

void UMASkillDefinition::InitializeRuntimeState(UMASkillAbility* SkillAbility)
{
	if (!SkillAbility || bRuntimeInitialized) return;

	for (int32 StepIndex = 0; StepIndex < SkillSteps.Num(); ++StepIndex)
	{
		UMASkillStep* RuntimeStep = SkillSteps[StepIndex];
		if (!RuntimeStep) continue;

		const int32 NextStepIndex = SkillSteps.IsValidIndex(StepIndex + 1) ? StepIndex + 1 : INDEX_NONE;
		const int32 NextMontageStepIndex = ResolveNextMontageStepIndex(SkillSteps, StepIndex);
		RuntimeStep->InitializeStep(SkillAbility, StepIndex, NextStepIndex, NextMontageStepIndex);
	}

	CollectEventActions(ResolvedRequiredEventTags, ResolvedActionsByEvent);
	bRuntimeInitialized = true;
}

void UMASkillDefinition::RebindEventTasks()
{
	if (!OwnerSkillAbility) return;

	for (UAbilityTask_WaitGameplayEvent* Task : EventTasks)
	{
		if (Task) Task->EndTask();
	}

	EventTasks.Reset();

	TSet<FGameplayTag> RequiredTags = ResolvedRequiredEventTags;
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->CollectRequiredEventTags(RequiredTags);
	}

	for (const FGameplayTag& EventTag : RequiredTags)
	{
		UAbilityTask_WaitGameplayEvent* WaitGameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkillAbility, EventTag, nullptr, false, false);
		if (!WaitGameplayEventTask) continue;

		WaitGameplayEventTask->EventReceived.AddDynamic(this, &UMASkillDefinition::HandleBoundGameplayEvent);
		WaitGameplayEventTask->ReadyForActivation();
		EventTasks.Add(WaitGameplayEventTask);
	}
}

void UMASkillDefinition::EnterCurrentStep()
{
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->EnterStep(CurrentStepStartMode);
	}
}

void UMASkillDefinition::ClearPreparedStepPreviews()
{
	for (UMASkillStep* RuntimeSkillStep : SkillSteps)
	{
		if (!RuntimeSkillStep) continue;
		RuntimeSkillStep->ClearPreparedStepPreview();
	}
}

UMASkillStep* UMASkillDefinition::GetRuntimeSkillStep(int32 StepIndex) const
{
	return SkillSteps.IsValidIndex(StepIndex) ? SkillSteps[StepIndex] : nullptr;
}

UMASkillStep* UMASkillDefinition::GetCurrentRuntimeSkillStep() const
{
	return GetRuntimeSkillStep(CurrentStepIndex);
}

void UMASkillDefinition::EndOwningSkillAbility()
{
	if (OwnerSkillAbility)
	{
		OwnerSkillAbility->K2_EndAbility();
	}
}

void UMASkillDefinition::HandleBoundGameplayEvent(FGameplayEventData Payload)
{
	if (!OwnerSkillAbility) return;

	HandleSkillGameplayEvent(Payload, OwnerSkillAbility->GetRuntimeContext(), OwnerSkillAbility->GetPayloadStore());
}
