#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/MASkillGameplayEventPart.h"
#include "GAS/Skill/Event/MASkillEventSource.h"
#include "GAS/Skill/Step/MASkillStepManager.h"

UMASkillAbility::UMASkillAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStunStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetAirborneStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetGrabStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStaggerStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetKnockbackStatTag());
}

void UMASkillAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	UMASkillDefinition* SourceSkillDefinition = Cast<UMASkillDefinition>(Spec.SourceObject.Get());
	if (!SourceSkillDefinition) SourceSkillDefinition = SkillDefinition;
	UpdateCurrentSkillDefinition(SourceSkillDefinition);
}

void UMASkillAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);
	UpdateCurrentSkillDefinition(nullptr);
}

void UMASkillAbility::UpdateCurrentSkillDefinition(UMASkillDefinition* SourceSkillDefinition)
{
	// 런타임 데피니션 수정 불가. 나중에 가능하게 되면, 차라리 액티베이션 종료 후 반영하도록 바인딩 하면 될 듯.
	if (!ensureMsgf(!IsActive(), TEXT("UpdateCurrentSkillDefinition must not run while the skill is active."))) return;

	UnbindGameplayEvents();
	if (CurrentSkillDefinition)
	{
		for (UMASkillEventSource* RuntimeEventSource : CurrentSkillDefinition->GetEventSources())
		{
			if (!RuntimeEventSource) continue;
			RuntimeEventSource->DeinitializeRuntime();
		}
	}
	if (StepManager) StepManager->ResetRuntimeState();

	CurrentSkillDefinition = SourceSkillDefinition
		? DuplicateObject<UMASkillDefinition>(SourceSkillDefinition, this)
		: nullptr;

	if (!CurrentSkillDefinition) return;
	EnsureStepManager();
	StepManager->UpdateSteps(CurrentSkillDefinition->GetSkillSteps());
	EnsureEventSources();
}

void UMASkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CurrentSkillDefinition) { K2_EndAbility(); return; }
	if (!K2_CommitAbility()) { K2_EndAbility(); return; }

	PayloadStore.Reset();
	CurrentSkillDefinition->ApplyPayloadsTo(PayloadStore);
	BindGameplayEvents();
	SkillActivatedDelegate.Broadcast();
	RegisterCancelTriggers();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMASkillAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	SkillDeactivatedDelegate.Broadcast();
	UnbindGameplayEvents();

	UnregisterCancelTriggers();
	if (AMACharacter* OwnerCharacter = Cast<AMACharacter>(GetAvatarActorFromActorInfo()))
	{
		if (UMAImpulseComponent* ImpulseComponent = OwnerCharacter->GetImpulseComponent())
		{
			ImpulseComponent->StopOwnedActionImpulses(this);
		}
	}

	PayloadStore.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const FGameplayTag& UMASkillAbility::GetElementalTag() const
{
	static const FGameplayTag EmptyTag;
	if (!CurrentSkillDefinition) return EmptyTag;
	return CurrentSkillDefinition->GetElementalTag();
}

void UMASkillAbility::HandleSkillGameplayEvent(FGameplayEventData Payload)
{
	if (StepManager) StepManager->HandleRuntimeEvent(Payload);
	if (!CurrentSkillDefinition || !Payload.EventTag.IsValid()) return;

	for (const FMASkillGameplayEventPart& EventPart : CurrentSkillDefinition->GetEventParts())
	{
		if (EventPart.EventTag != Payload.EventTag || !EventPart.Action)
			continue;

		EventPart.Action->Execute(*this, Payload);
	}
}

bool UMASkillAbility::CanPlaySkillMontageLocally() const
{
	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	return ActorInfo
		&& (HasAuthorityOrPredictionKey(ActorInfo, &CurrentActivationInfo) || ActorInfo->IsLocallyControlled());
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
	if (!IsActive()) return;

	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UMASkillAbility::BindGameplayEvents()
{
	if (!CurrentSkillDefinition || !EventTasks.IsEmpty()) return;

	TSet<FGameplayTag> RequiredEventTags;
	for (const FMASkillGameplayEventPart& EventPart : CurrentSkillDefinition->GetEventParts())
	{
		if (!EventPart.EventTag.IsValid() || !EventPart.Action)
			continue;

		RequiredEventTags.Add(EventPart.EventTag);
	}

	for (const FGameplayTag& EventTag : RequiredEventTags)
	{
		if (!EventTag.IsValid()) continue;

		UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag, nullptr, false, false);
		if (!EventTask) continue;

		EventTask->EventReceived.AddDynamic(this, &UMASkillAbility::HandleSkillGameplayEvent);
		EventTask->ReadyForActivation();
		EventTasks.Add(EventTask);
	}
}

void UMASkillAbility::UnbindGameplayEvents()
{
	for (UAbilityTask_WaitGameplayEvent* EventTask : EventTasks)
	{
		if (!EventTask) continue;

		EventTask->EventReceived.Clear();
		EventTask->EndTask();
	}

	EventTasks.Reset();
}

void UMASkillAbility::EnsureStepManager()
{
	if (!CurrentSkillDefinition) return;

	if (!StepManager)
	{
		StepManager = NewObject<UMASkillStepManager>(this);
		StepManager->Initialize(this);
	}
}

void UMASkillAbility::EnsureEventSources()
{
	if (!CurrentSkillDefinition) return;

	for (UMASkillEventSource* RuntimeEventSource : CurrentSkillDefinition->GetEventSources())
	{
		if (!RuntimeEventSource) continue;

		RuntimeEventSource->InitializeRuntime(this);
	}
}
