#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"

namespace
{
	template <typename TaskType>
	void EndAbilityTasksAndReset(TArray<TObjectPtr<TaskType>>& Tasks)
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

	RuntimeContext.Initialize(this);
	RegisterFlowPart();
	RefreshEventBindings();

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

	EndAbilityTasksAndReset(EventTasks);
	RuntimeContext.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMASkillAbility::HandleSkillGameplayEvent(FGameplayEventData Payload)
{
	RuntimeContext.HandleEvent(Payload);
}

void UMASkillAbility::RegisterFlowPart()
{
	if (!SkillDefinition || !SkillDefinition->GetFlowPart()) return;

	RuntimeFlowPart = DuplicateObject<UMASkillFlowPart>(SkillDefinition->GetFlowPart(), this);
	if (RuntimeFlowPart)
	{
		RuntimeFlowPart->StartFlow(this, &RuntimeContext);
	}
}

void UMASkillAbility::RefreshEventBindings()
{
	EndAbilityTasksAndReset(EventTasks);
	const TSet<FGameplayTag> RequiredTags = RuntimeContext.ResolveRequiredEventTags();
	for (const FGameplayTag& EventTag : RequiredTags)
	{
		UAbilityTask_WaitGameplayEvent* WaitGameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag, nullptr, false, false);
		WaitGameplayEventTask->EventReceived.AddDynamic(this, &UMASkillAbility::HandleSkillGameplayEvent);
		WaitGameplayEventTask->ReadyForActivation();
		EventTasks.Add(WaitGameplayEventTask);
	}
}
