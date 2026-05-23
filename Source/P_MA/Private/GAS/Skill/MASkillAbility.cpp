#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/Binding/MASkillGameplayEventBinding.h"
#include "GAS/Skill/Event/Publish/MASkillGameplayEventScope.h"
#include "GAS/Skill/Event/Publish/MASkillEventSource.h"
#include "GAS/Skill/MASkillGenericDataAsset.h"
#include "GAS/Skill/MASkillManagerComponent.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Step/MASkillStepManager.h"

UMASkillAbility::UMASkillAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	const FGameplayTag SkillTag = FGameplayTag::RequestGameplayTag(TEXT("Skill"));
	AbilityTags.AddTag(SkillTag);
	BlockAbilitiesWithTag.AddTag(SkillTag);

	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStunStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetAirborneStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetGrabStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetStaggerStatTag());
	CancelTriggerTags.AddTag(UMAAbilitySystemStatics::GetKnockbackStatTag());
}

void UMASkillAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	if (const AMACharacter* OwnerCharacter = Cast<AMACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr))
	{
		if (UMASkillManagerComponent* SkillManager = OwnerCharacter->GetSkillManagerComponent())
		{
			SkillManager->RegisterAbilityHandle(static_cast<EMAAbilityInputID>(Spec.InputID), Spec.Handle, GetClass());
		}
	}
}

void UMASkillAbility::OnRemoveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnRemoveAbility(ActorInfo, Spec);

	if (const AMACharacter* OwnerCharacter = Cast<AMACharacter>(ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr))
	{
		if (UMASkillManagerComponent* SkillManager = OwnerCharacter->GetSkillManagerComponent())
		{
			SkillManager->UnregisterAbilityHandle(static_cast<EMAAbilityInputID>(Spec.InputID), Spec.Handle);
		}
	}

	UpdateCurrentSkillModuleInstance(nullptr);
}

const UMASkillDefinition* UMASkillAbility::GetCurrentSkillDefinition() const
{
	return CurrentSkillModuleInstance ? CurrentSkillModuleInstance->GetDefinition() : nullptr;
}

FMASkillPayloadStore* UMASkillAbility::GetModulePayloadStore(UMASkillModuleInstance* RuntimeScope) const
{
	return RuntimeScope ? &RuntimeScope->GetPayloadStore() : nullptr;
}

FMASkillPayloadStore& UMASkillAbility::GetAssembledModulePayloadStore()
{
	check(CurrentSkillModuleInstance);
	return CurrentSkillModuleInstance->GetPayloadStore();
}

void UMASkillAbility::UpdateCurrentSkillModuleInstance(UMASkillModuleInstance* SourceSkillModuleInstance)
{
	if (IsActive())
	{
		PendingSkillModuleInstance = SourceSkillModuleInstance;
		bHasPendingSkillModuleInstanceUpdate = true;
		return;
	}

	PendingSkillModuleInstance = nullptr;
	bHasPendingSkillModuleInstanceUpdate = false;
	ApplyCurrentSkillModuleInstance(SourceSkillModuleInstance);
}

void UMASkillAbility::ApplyCurrentSkillModuleInstance(UMASkillModuleInstance* SourceSkillModuleInstance)
{
	UnbindGameplayEvents();
	if (const UMASkillDefinition* CurrentSkillDefinition = GetCurrentSkillDefinition())
	{
		for (UMASkillEventSource* RuntimeEventSource : CurrentSkillDefinition->GetEventSources())
		{
			if (!RuntimeEventSource) continue;
			RuntimeEventSource->DeinitializeRuntime();
		}
	}
	if (StepManager) StepManager->ResetRuntimeState();

	CurrentSkillModuleInstance = SourceSkillModuleInstance;
	const UMASkillDefinition* CurrentSkillDefinition = GetCurrentSkillDefinition();
	if (!CurrentSkillDefinition)
	{
		return;
	}

	EnsureStepManager();
	StepManager->UpdateSteps(CurrentSkillDefinition->GetSkillSteps());
	EnsureEventSources();
}

void UMASkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	const UMASkillDefinition* CurrentSkillDefinition = GetCurrentSkillDefinition();
	if (!CurrentSkillDefinition) { K2_EndAbility(); return; }
	if (!K2_CommitAbility()) { K2_EndAbility(); return; }

	FMASkillPayloadStore& AssembledModulePayloadStore = GetAssembledModulePayloadStore();
	AssembledModulePayloadStore.Reset();
	CurrentSkillDefinition->ApplyPayloadsTo(AssembledModulePayloadStore);
	if (StepManager)
	{
		StepManager->SetDesiredMontagePlayRate(1.f);
	}
	BindGameplayEvents();
	RegisterCancelTriggers();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	SkillActivatedDelegate.Broadcast();

	if (IsActive() && (!StepManager || !StepManager->GetCurrentRuntimeSkillStep()))
	{
		K2_EndAbility();
	}
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

	if (CurrentSkillModuleInstance)
	{
		CurrentSkillModuleInstance->ResetPayloadStore();
	}
	if (bHasPendingSkillModuleInstanceUpdate)
	{
		UMASkillModuleInstance* NextSkillModuleInstance = PendingSkillModuleInstance;
		PendingSkillModuleInstance = nullptr;
		bHasPendingSkillModuleInstanceUpdate = false;
		ApplyCurrentSkillModuleInstance(NextSkillModuleInstance);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const FGameplayTag& UMASkillAbility::GetElementalTag() const
{
	static const FGameplayTag EmptyTag;
	static const FGameplayTag DefaultElementalTag = UMAAbilitySystemStatics::GetDefaultElementalTag();
	const UMASkillDefinition* CurrentSkillDefinition = GetCurrentSkillDefinition();
	if (!CurrentSkillDefinition) return EmptyTag;

	const FGameplayTag& CurrentElementalTag = CurrentSkillDefinition->GetElementalTag();
	return CurrentElementalTag.IsValid()
		? CurrentElementalTag
		: DefaultElementalTag;
}

const UDataTable* UMASkillAbility::GetElementalDataTable() const
{
	const UMASkillGenericDataAsset* GenericSkillDataAsset = GetGenericSkillDataAsset();
	return GenericSkillDataAsset ? GenericSkillDataAsset->GetElementalDataTable() : nullptr;
}

const UDataTable* UMASkillAbility::GetOverlapDecalDataTable() const
{
	const UMASkillGenericDataAsset* GenericSkillDataAsset = GetGenericSkillDataAsset();
	return GenericSkillDataAsset ? GenericSkillDataAsset->GetOverlapDecalDataTable() : nullptr;
}

const UMASkillGenericDataAsset* UMASkillAbility::GetGenericSkillDataAsset() const
{
	const AMACharacter* OwnerCharacter = Cast<AMACharacter>(GetAvatarActorFromActorInfo());
	if (!OwnerCharacter) return nullptr;

	const UMASkillManagerComponent* SkillManager = OwnerCharacter->GetSkillManagerComponent();
	return SkillManager ? SkillManager->GetGenericSkillDataAsset() : nullptr;
}

void UMASkillAbility::HandleExternalGameplayEvent(FGameplayEventData Payload)
{
	UMASkillModuleInstance* RuntimeScope = MASkillGameplayEventScope::ExtractRuntimeScope(Payload);

	if (StepManager) StepManager->HandleRuntimeEvent(Payload);
	const UMASkillDefinition* CurrentSkillDefinition = GetCurrentSkillDefinition();
	if (!CurrentSkillDefinition || !Payload.EventTag.IsValid()) return;

	for (const FMASkillGameplayEventBinding& EventBinding : CurrentSkillDefinition->GetEventBindings())
	{
		if (EventBinding.EventTag != Payload.EventTag || !EventBinding.Action)
			continue;
		if (EventBinding.bUseLocalBinding)
		{
			if (!EventBinding.RuntimeScope || EventBinding.RuntimeScope != RuntimeScope)
			{
				continue;
			}
		}

		EventBinding.Action->Execute(*this, Payload, RuntimeScope);
	}
}

void UMASkillAbility::SendSkillGameplayEvent(const FGameplayEventData& Payload, UMASkillModuleInstance* RuntimeScope)
{
	AActor* TargetActor = GetAvatarActorFromActorInfo();
	if (!TargetActor || !Payload.EventTag.IsValid()) return;

	FGameplayEventData EventPayload = Payload;
	MASkillGameplayEventScope::InjectRuntimeScope(EventPayload, RuntimeScope);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, EventPayload.EventTag, EventPayload);
}

UMASkillModuleInstance* UMASkillAbility::GetCurrentRuntimeScope() const
{
	return StepManager
		? StepManager->GetCurrentRuntimeScope()
		: nullptr;
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
	const UMASkillDefinition* CurrentSkillDefinition = GetCurrentSkillDefinition();
	if (!CurrentSkillDefinition || !EventTasks.IsEmpty()) return;

	TSet<FGameplayTag> RequiredEventTags;
	for (const FMASkillGameplayEventBinding& EventBinding : CurrentSkillDefinition->GetEventBindings())
	{
		if (!EventBinding.EventTag.IsValid() || !EventBinding.Action)
			continue;

		RequiredEventTags.Add(EventBinding.EventTag);
	}

	for (const FGameplayTag& EventTag : RequiredEventTags)
	{
		if (!EventTag.IsValid()) continue;

		UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag, nullptr, false, true);
		if (!EventTask) continue;

		EventTask->EventReceived.AddDynamic(this, &UMASkillAbility::HandleExternalGameplayEvent);
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
	const UMASkillDefinition* CurrentSkillDefinition = GetCurrentSkillDefinition();
	if (!CurrentSkillDefinition) return;

	if (!StepManager)
	{
		StepManager = NewObject<UMASkillStepManager>(this);
		StepManager->Initialize(this);
	}
}

void UMASkillAbility::EnsureEventSources()
{
	const UMASkillDefinition* CurrentSkillDefinition = GetCurrentSkillDefinition();
	if (!CurrentSkillDefinition) return;

	for (UMASkillEventSource* RuntimeEventSource : CurrentSkillDefinition->GetEventSources())
	{
		if (!RuntimeEventSource) continue;

		RuntimeEventSource->InitializeRuntime(this);
	}
}
