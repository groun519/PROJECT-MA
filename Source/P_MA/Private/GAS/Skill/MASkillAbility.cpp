#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Action/MASkillAction.h"
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
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetAirborneStatTag());
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
	RegisterFlowParts();
	StartCurrentFlow();
	RegisterEventSources();
	RefreshEventBindings();
	RegisterCancelTriggers();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMASkillAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UnregisterFlowParts();
	UnregisterEventSources();
	EndAbilityTasksAndReset(EventTasks);
	UnregisterCancelTriggers();
	if (AMACharacter* OwnerCharacter = Cast<AMACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UMAImpulseComponent* ImpulseComponent = OwnerCharacter->GetImpulseComponent())
		{
			ImpulseComponent->StopOwnedActionImpulses(this);
		}
	}
	RuntimeContext.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UMASkillAbility::HandleSkillTagEvent(const FGameplayTag& EventTag)
{
	if (!EventTag.IsValid()) return;

	FGameplayEventData Payload;
	Payload.EventTag = EventTag;
	HandleSkillGameplayEvent(Payload);
}

void UMASkillAbility::HandleSkillGameplayEvent(FGameplayEventData Payload)
{
	HandleCurrentFlowRuntimeEvent(Payload);
	RuntimeContext.RefreshStateFromEvent(Payload);

	TArray<UMASkillAction*> ResolvedActions;
	RuntimeContext.ResolveActionsForEvent(Payload, ResolvedActions);
	for (UMASkillAction* Action : ResolvedActions)
	{
		if (!Action) continue;
		Action->Execute(*this, RuntimeContext, Payload);
	}
}

UMASkillFlowPart* UMASkillAbility::GetCurrentRuntimeFlowPart() const
{
	return RuntimeFlowParts.IsValidIndex(CurrentFlowIndex) ? RuntimeFlowParts[CurrentFlowIndex] : nullptr;
}

void UMASkillAbility::SetDesiredMontagePlayRate(float NewPlayRate)
{
	DesiredMontagePlayRate = FMath::Max(NewPlayRate, KINDA_SMALL_NUMBER);

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	UAnimMontage* FlowMontage = CurrentFlowPart ? CurrentFlowPart->ResolveFlowMontage() : nullptr;
	if (!AnimInstance || !FlowMontage) return;

	if (AnimInstance->Montage_IsPlaying(FlowMontage))
	{
		AnimInstance->Montage_SetPlayRate(FlowMontage, DesiredMontagePlayRate);
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

		RuntimeEventSource->StartSource(this);
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

void UMASkillAbility::RegisterFlowParts()
{
	UnregisterFlowParts();
	if (!SkillDefinition) return;

	for (UMASkillFlowPart* FlowPart : SkillDefinition->GetFlowParts())
	{
		if (!FlowPart) continue;

		UMASkillFlowPart* RuntimeFlowPart = DuplicateObject<UMASkillFlowPart>(FlowPart, this);
		if (!RuntimeFlowPart) continue;
		RuntimeFlowParts.Add(RuntimeFlowPart);
	}

	CurrentFlowIndex = RuntimeFlowParts.IsEmpty() ? INDEX_NONE : 0;
}

void UMASkillAbility::UnregisterFlowParts()
{
	if (UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart())
	{
		CurrentFlowPart->StopFlow();
	}

	RuntimeFlowParts.Reset();
	CurrentFlowIndex = INDEX_NONE;
}

void UMASkillAbility::StartCurrentFlow()
{
	UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	if (!CurrentFlowPart) return;

	CurrentFlowPart->StartFlow(this);

	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	UAnimMontage* FlowMontage = CurrentFlowPart->ResolveFlowMontage();
	if (!ActorInfo || !HasAuthorityOrPredictionKey(ActorInfo, &CurrentActivationInfo) || !FlowMontage) return;

	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FlowMontage, DesiredMontagePlayRate);
	if (!CurrentMontageTask) return;

	CurrentMontageTask->OnCancelled.AddDynamic(this, &UMASkillAbility::HandleCurrentFlowMontageCancelled);
	CurrentMontageTask->OnCompleted.AddDynamic(this, &UMASkillAbility::HandleCurrentFlowMontageCompleted);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UMASkillAbility::HandleCurrentFlowMontageInterrupted);
	CurrentMontageTask->ReadyForActivation();
}

bool UMASkillAbility::AdvanceToNextFlow()
{
	UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	if (CurrentFlowPart)
	{
		CurrentFlowPart->StopFlow();
	}

	const int32 NextFlowIndex = CurrentFlowIndex + 1;
	if (!RuntimeFlowParts.IsValidIndex(NextFlowIndex))
	{
		CurrentFlowIndex = INDEX_NONE;
		return false;
	}

	CurrentFlowIndex = NextFlowIndex;
	StartCurrentFlow();
	RefreshEventBindings();
	return true;
}

void UMASkillAbility::HandleCurrentFlowRuntimeEvent(const FGameplayEventData& Payload)
{
	if (UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart())
	{
		CurrentFlowPart->HandleRuntimeEvent(Payload);
	}
}

void UMASkillAbility::RefreshEventBindings()
{
	EndAbilityTasksAndReset(EventTasks);
	TSet<FGameplayTag> RequiredTags = RuntimeContext.ResolveRequiredEventTags();
	if (UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart())
	{
		CurrentFlowPart->CollectRequiredEventTags(RequiredTags);
	}
	for (const FGameplayTag& EventTag : RequiredTags)
	{
		UAbilityTask_WaitGameplayEvent* WaitGameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag, nullptr, false, false);
		WaitGameplayEventTask->EventReceived.AddDynamic(this, &UMASkillAbility::HandleSkillGameplayEvent);
		WaitGameplayEventTask->ReadyForActivation();
		EventTasks.Add(WaitGameplayEventTask);
	}
}

void UMASkillAbility::HandleCurrentFlowMontageCancelled()
{
	CurrentMontageTask = nullptr;
	K2_EndAbility();
}

void UMASkillAbility::HandleCurrentFlowMontageCompleted()
{
	CurrentMontageTask = nullptr;
	if (!AdvanceToNextFlow())
	{
		K2_EndAbility();
	}
}

void UMASkillAbility::HandleCurrentFlowMontageInterrupted()
{
	CurrentMontageTask = nullptr;
	K2_EndAbility();
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
