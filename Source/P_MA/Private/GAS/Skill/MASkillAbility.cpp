#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/MASkillEventSource.h"
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

UMASkillAbility::UMASkillAbility()
{
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStunStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetGrabStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStaggerStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetKnockbackStatTag());
}

void UMASkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!SkillDefinition) { K2_EndAbility(); return; }

	if (!K2_CommitAbility()) { K2_EndAbility(); return; }

	RuntimeContext.Initialize(this);
	DesiredMontagePlayRate = 1.f;
	RegisterFlowPart();
	RegisterEventSources();
	RefreshEventBindings();
	RegisterCancelTriggers();

	if (UAnimMontage* SkillMontage = SkillDefinition->GetSkillMontage(); HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo) && SkillMontage)
	{
		UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, SkillMontage, DesiredMontagePlayRate);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UMASkillAbility::K2_EndAbility);
		PlayMontageTask->OnCompleted.AddDynamic(this, &UMASkillAbility::K2_EndAbility);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UMASkillAbility::K2_EndAbility);
		PlayMontageTask->ReadyForActivation();
	}

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

	UnregisterEventSources();
	EndAbilityTasksAndReset(EventTasks);
	UnregisterCancelTriggers();
	RuntimeContext.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMASkillAbility::HandleSkillGameplayEvent(FGameplayEventData Payload)
{
	RuntimeContext.HandleEvent(Payload);
}

void UMASkillAbility::SetDesiredMontagePlayRate(float NewPlayRate)
{
	DesiredMontagePlayRate = FMath::Max(NewPlayRate, KINDA_SMALL_NUMBER);

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UAnimMontage* SkillMontage = SkillDefinition ? SkillDefinition->GetSkillMontage() : nullptr;
	if (!AnimInstance || !SkillMontage) return;

	if (AnimInstance->Montage_IsPlaying(SkillMontage))
	{
		AnimInstance->Montage_SetPlayRate(SkillMontage, DesiredMontagePlayRate);
	}
}

void UMASkillAbility::RegisterEventSources()
{
	UnregisterEventSources();

	if (!SkillDefinition) return;

	for (UMASkillEventSource* EventSource : SkillDefinition->GetEventSources())
	{
		if (!EventSource) continue;

		UMASkillEventSource* RuntimeEventSource = DuplicateObject<UMASkillEventSource>(EventSource, this);
		if (!RuntimeEventSource) continue;

		RuntimeEventSource->StartSource(this, &RuntimeContext);
		RuntimeEventSources.Add(RuntimeEventSource);
	}
}

void UMASkillAbility::UnregisterEventSources()
{
	for (UMASkillEventSource* RuntimeEventSource : RuntimeEventSources)
	{
		if (!RuntimeEventSource) continue;
		RuntimeEventSource->StopSource();
	}

	RuntimeEventSources.Reset();
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

void UMASkillAbility::RegisterCancelTriggers()
{
	UnregisterCancelTriggers();

	if (CancelTriggerTags.IsEmpty()) return;

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC) return;

	for (const FGameplayTag& CancelTriggerTag : CancelTriggerTags)
	{
		if (!CancelTriggerTag.IsValid()) continue;

		FDelegateHandle DelegateHandle = ASC->RegisterGameplayTagEvent(CancelTriggerTag).AddUObject(this, &UMASkillAbility::HandleCancelTriggerTagChanged);
		CancelTriggerDelegateHandles.Add(CancelTriggerTag, DelegateHandle);
	}
}

void UMASkillAbility::UnregisterCancelTriggers()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		CancelTriggerDelegateHandles.Reset();
		return;
	}

	for (const TPair<FGameplayTag, FDelegateHandle>& DelegatePair : CancelTriggerDelegateHandles)
	{
		ASC->RegisterGameplayTagEvent(DelegatePair.Key).Remove(DelegatePair.Value);
	}

	CancelTriggerDelegateHandles.Reset();
}

void UMASkillAbility::HandleCancelTriggerTagChanged(FGameplayTag Tag, int32 NewCount)
{
	if (NewCount != 1) return;
	if (!CancelTriggerTags.HasTagExact(Tag)) return;
	if (!IsActive()) return;

	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}
