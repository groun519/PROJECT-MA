#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"

namespace
{
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
}

void UMASkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!SkillDefinition)
	{
		K2_EndAbility();
		return;
	}

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	if (UAnimMontage* SkillMontage = SkillDefinition->GetSkillMontage(); HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) && SkillMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UMASkillAbility::K2_EndAbility);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UMASkillAbility::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UMASkillAbility::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}

	RuntimeContext.ClearIgnoredActors();
	RegisterEventParts();
	RegisterFlowPart();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMASkillAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (RuntimeFlowPart)
	{
		RuntimeFlowPart->StopFlow();
		RuntimeFlowPart = nullptr;
	}

	EndTasksAndReset(EventTasks);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMASkillAbility::HandleSkillGameplayEvent(FGameplayEventData Payload)
{
	if (!SkillDefinition) return;

	for (const FMASkillGameplayEventPart& EventPart : SkillDefinition->GetEventParts())
	{
		if (EventPart.EventTag != Payload.EventTag || !EventPart.Action) continue;
		
		EventPart.Action->Execute(this, RuntimeContext, Payload);
	}
}

void UMASkillAbility::RegisterFlowPart()
{
	if (!SkillDefinition || !SkillDefinition->GetFlowPart()) return;

	RuntimeFlowPart = DuplicateObject<UMASkillFlowPart>(SkillDefinition->GetFlowPart(), this);
	if (RuntimeFlowPart)
	{
		RuntimeFlowPart->StartFlow(this);
	}
}

void UMASkillAbility::RegisterEventParts()
{
	EndTasksAndReset(EventTasks);
	TSet<FGameplayTag> RegisteredTags;

	for (const FMASkillGameplayEventPart& EventPart : SkillDefinition->GetEventParts())
	{
		if (!EventPart.EventTag.IsValid() || !EventPart.Action) continue;

		if (RegisteredTags.Contains(EventPart.EventTag)) continue;
		RegisteredTags.Add(EventPart.EventTag);

		UAbilityTask_WaitGameplayEvent* WaitGameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventPart.EventTag, nullptr, false, false);
		WaitGameplayEventTask->EventReceived.AddDynamic(this, &UMASkillAbility::HandleSkillGameplayEvent);
		WaitGameplayEventTask->ReadyForActivation();
		EventTasks.Add(WaitGameplayEventTask);
	}
}
