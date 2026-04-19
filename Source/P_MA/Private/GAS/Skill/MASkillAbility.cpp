#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Animation/MAAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/MASkillEventSource.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"

namespace
{
	constexpr float PreparedPreviewPlayRate = KINDA_SMALL_NUMBER;

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
	CacheRuntimeSkillDefinition(Handle, ActorInfo);
	if (!GetSkillDefinition()) { K2_EndAbility(); return; }

	if (!K2_CommitAbility()) { K2_EndAbility(); return; }

	ResetResolvedData();
	ResolvePayloads();
	ResolveEventActions();
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
	ResetResolvedData();
	RuntimeSkillDefinition = nullptr;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

const UMASkillDefinition* UMASkillAbility::GetSkillDefinition() const
{
	return RuntimeSkillDefinition ? RuntimeSkillDefinition : SkillDefinition;
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
	if (UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart())
	{
		CurrentFlowPart->HandleRuntimeEvent(Payload);
	}

	TArray<UMASkillAction*> ResolvedActions;
	ResolveActionsForEvent(Payload.EventTag, ResolvedActions);
	for (UMASkillAction* Action : ResolvedActions)
	{
		if (!Action) continue;
		Action->Execute(*this, RuntimeContext, PayloadStore, Payload);
	}
}

const FGameplayTag& UMASkillAbility::GetElementalTag() const
{
	static const FGameplayTag EmptyTag;
	return GetSkillDefinition() ? GetSkillDefinition()->GetElementalTag() : EmptyTag;
}

UMASkillFlowPart* UMASkillAbility::GetCurrentRuntimeFlowPart() const
{
	return RuntimeFlowParts.IsValidIndex(CurrentFlowIndex) ? RuntimeFlowParts[CurrentFlowIndex] : nullptr;
}

void UMASkillAbility::SetDesiredMontagePlayRate(float NewPlayRate)
{
	DesiredMontagePlayRate = FMath::Max(NewPlayRate, 0.f);

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	UAnimMontage* FlowMontage = CurrentFlowPart ? CurrentFlowPart->ResolveFlowMontage() : nullptr;
	if (!AnimInstance || !FlowMontage) return;

	if (AnimInstance->Montage_IsPlaying(FlowMontage))
	{
		AnimInstance->Montage_SetPlayRate(FlowMontage, DesiredMontagePlayRate);
	}
}

void UMASkillAbility::CompleteCurrentFlow(float MontageBlendOutTime)
{
	if (!AdvanceToNextFlow(MontageBlendOutTime))
	{
		K2_EndAbility();
	}
}

bool UMASkillAbility::PrepareNextFlowMontage(float PreviewBlendInTime)
{
	if (CurrentFlowStartMode != EMASkillFlowStartMode::Fresh) return false;
	if (PreparedMontage || PreparedFlowIndex != INDEX_NONE) return false;

	UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	if (CurrentFlowPart && CurrentFlowPart->ResolveFlowMontage()) return false;

	const int32 NextFlowIndex = CurrentFlowIndex + 1;
	if (!RuntimeFlowParts.IsValidIndex(NextFlowIndex)) return false;

	UMASkillFlowPart* NextFlowPart = RuntimeFlowParts[NextFlowIndex];
	UAnimMontage* NextFlowMontage = NextFlowPart ? NextFlowPart->ResolveFlowMontage() : nullptr;
	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!ActorInfo || !HasAuthorityOrPredictionKey(ActorInfo, &CurrentActivationInfo) || !AnimInstance || !NextFlowMontage) return false;

	if (AnimInstance->Montage_PlayWithBlendSettings(NextFlowMontage, FMontageBlendSettings(FMath::Max(PreviewBlendInTime, 0.f)), PreparedPreviewPlayRate) <= 0.f) return false;

	RegisterAnimationOwner(NextFlowMontage);
	BindPreparedMontageDelegates(NextFlowMontage);
	PreparedMontage = NextFlowMontage;
	PreparedFlowIndex = NextFlowIndex;
	return true;
}

bool UMASkillAbility::ActivatePreparedNextFlow()
{
	if (!RuntimeFlowParts.IsValidIndex(PreparedFlowIndex) || !PreparedMontage) return false;

	UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	if (CurrentFlowPart)
	{
		CurrentFlowPart->StopFlow();
	}

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		if (FAnimMontageInstance* PreparedMontageInstance = AnimInstance->GetInstanceForMontage(PreparedMontage))
		{
			PreparedMontageInstance->SetWeight(1.f);
			PreparedMontageInstance->SetDesiredWeight(1.f);
		}
	}

	CurrentFlowIndex = PreparedFlowIndex;
	PreparedFlowIndex = INDEX_NONE;
	CurrentFlowStartMode = EMASkillFlowStartMode::Prepared;
	CurrentMontageTask = nullptr;
	PreparedMontage = nullptr;

	StartCurrentFlow();
	RefreshEventBindings();
	SetDesiredMontagePlayRate(DesiredMontagePlayRate);
	return true;
}

void UMASkillAbility::RegisterEventSources()
{
	UnregisterEventSources();

	const UMASkillDefinition* ResolvedSkillDefinition = GetSkillDefinition();
	if (!ResolvedSkillDefinition) return;

	for (UMASkillEventSource* EventSource : ResolvedSkillDefinition->GetEventSources())
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
	const UMASkillDefinition* ResolvedSkillDefinition = GetSkillDefinition();
	if (!ResolvedSkillDefinition) return;

	for (UMASkillFlowPart* FlowPart : ResolvedSkillDefinition->GetFlowParts())
	{
		if (!FlowPart) continue;

		UMASkillFlowPart* RuntimeFlowPart = DuplicateObject<UMASkillFlowPart>(FlowPart, this);
		if (!RuntimeFlowPart) continue;
		RuntimeFlowParts.Add(RuntimeFlowPart);
	}

	CurrentFlowIndex = RuntimeFlowParts.IsEmpty() ? INDEX_NONE : 0;
	CurrentFlowStartMode = EMASkillFlowStartMode::Fresh;
	PreparedFlowIndex = INDEX_NONE;
}

void UMASkillAbility::UnregisterFlowParts()
{
	if (UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart())
	{
		CurrentFlowPart->StopFlow();
	}

	ClearCurrentMontageTask();
	StopCurrentFlowMontage();
	ClearPreparedMontage();
	RuntimeFlowParts.Reset();
	CurrentFlowIndex = INDEX_NONE;
	CurrentFlowStartMode = EMASkillFlowStartMode::Fresh;
	PreparedFlowIndex = INDEX_NONE;
}

void UMASkillAbility::StartCurrentFlow()
{
	UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	if (!CurrentFlowPart) return;

	const EMASkillFlowStartMode FlowStartMode = CurrentFlowStartMode;
	CurrentFlowPart->StartFlow(this, FlowStartMode);
	if (FlowStartMode == EMASkillFlowStartMode::Prepared) return;

	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	UAnimMontage* FlowMontage = CurrentFlowPart->ResolveFlowMontage();
	if (!ActorInfo || !HasAuthorityOrPredictionKey(ActorInfo, &CurrentActivationInfo) || !FlowMontage) return;

	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FlowMontage, DesiredMontagePlayRate);
	if (!CurrentMontageTask) return;

	CurrentMontageTask->OnCancelled.AddDynamic(this, &UMASkillAbility::HandleCurrentFlowMontageCancelled);
	CurrentMontageTask->OnCompleted.AddDynamic(this, &UMASkillAbility::HandleCurrentFlowMontageCompleted);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UMASkillAbility::HandleCurrentFlowMontageInterrupted);
	RegisterAnimationOwner(FlowMontage);
	CurrentMontageTask->ReadyForActivation();
}

bool UMASkillAbility::AdvanceToNextFlow(float CurrentFlowMontageBlendOutTime)
{
	UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	if (CurrentFlowPart)
	{
		CurrentFlowPart->StopFlow();
	}

	ClearCurrentMontageTask();
	StopCurrentFlowMontage(CurrentFlowMontageBlendOutTime);
	ClearPreparedMontage();

	const int32 NextFlowIndex = CurrentFlowIndex + 1;
	if (!RuntimeFlowParts.IsValidIndex(NextFlowIndex))
	{
		CurrentFlowIndex = INDEX_NONE;
		return false;
	}

	CurrentFlowIndex = NextFlowIndex;
	CurrentFlowStartMode = EMASkillFlowStartMode::Fresh;
	PreparedFlowIndex = INDEX_NONE;
	StartCurrentFlow();
	RefreshEventBindings();
	return true;
}

void UMASkillAbility::ClearCurrentMontageTask()
{
	if (!CurrentMontageTask) return;

	CurrentMontageTask->OnCancelled.Clear();
	CurrentMontageTask->OnCompleted.Clear();
	CurrentMontageTask->OnInterrupted.Clear();
	CurrentMontageTask->EndTask();
	CurrentMontageTask = nullptr;
}

void UMASkillAbility::StopCurrentFlowMontage(float MontageBlendOutTime)
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	UAnimMontage* FlowMontage = CurrentFlowPart ? CurrentFlowPart->ResolveFlowMontage() : nullptr;
	if (!AnimInstance || !FlowMontage) return;

	UnregisterAnimationOwner(FlowMontage);

	if (CurrentFlowStartMode == EMASkillFlowStartMode::Prepared)
	{
		ClearMontageDelegates(FlowMontage);
	}

	if (AnimInstance->Montage_IsPlaying(FlowMontage))
	{
		AnimInstance->Montage_Stop(MontageBlendOutTime, FlowMontage);
	}
}

void UMASkillAbility::ClearPreparedMontage()
{
	if (!PreparedMontage)
	{
		PreparedFlowIndex = INDEX_NONE;
		return;
	}

	UnregisterAnimationOwner(PreparedMontage);
	ClearMontageDelegates(PreparedMontage);
	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		if (AnimInstance->Montage_IsPlaying(PreparedMontage))
		{
			AnimInstance->Montage_Stop(0.f, PreparedMontage);
		}
	}

	PreparedMontage = nullptr;
	PreparedFlowIndex = INDEX_NONE;
}

void UMASkillAbility::RegisterAnimationOwner(UAnimSequenceBase* Animation)
{
	if (!Animation) return;

	if (UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(GetOwnerAnimInstance()))
	{
		AnimInstance->RegisterAnimationOwner(Animation, this);
	}
}

void UMASkillAbility::ResolvePayloads()
{
	const UMASkillDefinition* SkillDef = GetSkillDefinition();
	if (!SkillDef) return;

	for (const FMASkillPayloadEntry& Payload : SkillDef->GetPayloads())
	{
		Payload.ApplyTo(PayloadStore);
	}
}

void UMASkillAbility::ResolveEventActions()
{
	const UMASkillDefinition* SkillDef = GetSkillDefinition();
	if (!SkillDef) return;

	for (const FMASkillGameplayEventPart& EventPart : SkillDef->GetEventParts())
	{
		AddResolvedEventAction(EventPart.EventTag, EventPart.Action);
	}
}

void UMASkillAbility::ResetResolvedData()
{
	PayloadStore.Reset();
	ResolvedRequiredEventTags.Reset();
	ResolvedActionsByEvent.Reset();
}

void UMASkillAbility::AddResolvedEventAction(const FGameplayTag& EventTag, UMASkillAction* Action)
{
	if (!EventTag.IsValid() || !Action) return;

	ResolvedRequiredEventTags.Add(EventTag);
	ResolvedActionsByEvent.FindOrAdd(EventTag).Add(Action);
}

void UMASkillAbility::ResolveActionsForEvent(const FGameplayTag& EventTag, TArray<UMASkillAction*>& OutActions) const
{
	OutActions.Reset();

	if (const TArray<TObjectPtr<UMASkillAction>>* Actions = ResolvedActionsByEvent.Find(EventTag))
	{
		for (UMASkillAction* Action : *Actions)
		{
			if (Action)
			{
				OutActions.Add(Action);
			}
		}
	}
}

void UMASkillAbility::UnregisterAnimationOwner(UAnimSequenceBase* Animation)
{
	if (!Animation) return;

	if (UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(GetOwnerAnimInstance()))
	{
		AnimInstance->UnregisterAnimationOwner(Animation, this);
	}
}

void UMASkillAbility::RefreshEventBindings()
{
	EndAbilityTasksAndReset(EventTasks);
	TSet<FGameplayTag> RequiredTags = ResolvedRequiredEventTags;
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

void UMASkillAbility::BindPreparedMontageDelegates(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!AnimInstance || !Montage) return;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindUObject(this, &UMASkillAbility::HandlePreparedMontageBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);

	FOnMontageEnded EndedDelegate;
	EndedDelegate.BindUObject(this, &UMASkillAbility::HandlePreparedMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndedDelegate, Montage);
}

void UMASkillAbility::ClearMontageDelegates(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	if (!AnimInstance || !Montage) return;

	FOnMontageBlendingOutStarted EmptyBlendingOutDelegate;
	AnimInstance->Montage_SetBlendingOutDelegate(EmptyBlendingOutDelegate, Montage);

	FOnMontageEnded EmptyEndedDelegate;
	AnimInstance->Montage_SetEndDelegate(EmptyEndedDelegate, Montage);
}

void UMASkillAbility::HandlePreparedMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted) return;
	if (!Montage) return;

	const bool bIsPreparedPreview = Montage == PreparedMontage && PreparedFlowIndex != INDEX_NONE;
	const UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	const bool bIsPreparedCurrentFlow = CurrentFlowStartMode == EMASkillFlowStartMode::Prepared
		&& Montage == (CurrentFlowPart ? CurrentFlowPart->ResolveFlowMontage() : nullptr);
	if (!bIsPreparedPreview && !bIsPreparedCurrentFlow) return;

	ClearMontageDelegates(Montage);
	if (bIsPreparedPreview)
	{
		UnregisterAnimationOwner(Montage);
		PreparedMontage = nullptr;
		PreparedFlowIndex = INDEX_NONE;
	}
	HandleCurrentFlowMontageInterrupted();
}

void UMASkillAbility::HandlePreparedMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage || bInterrupted) return;

	const bool bIsPreparedPreview = Montage == PreparedMontage && PreparedFlowIndex != INDEX_NONE;
	const UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart();
	const bool bIsPreparedCurrentFlow = CurrentFlowStartMode == EMASkillFlowStartMode::Prepared
		&& Montage == (CurrentFlowPart ? CurrentFlowPart->ResolveFlowMontage() : nullptr);
	if (!bIsPreparedPreview && !bIsPreparedCurrentFlow) return;

	ClearMontageDelegates(Montage);
	if (bIsPreparedPreview)
	{
		UnregisterAnimationOwner(Montage);
		PreparedMontage = nullptr;
		PreparedFlowIndex = INDEX_NONE;
		K2_EndAbility();
		return;
	}

	HandleCurrentFlowMontageCompleted();
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

void UMASkillAbility::CacheRuntimeSkillDefinition(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo)
{
	RuntimeSkillDefinition = nullptr;

	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid()) return;

	const FGameplayAbilitySpec* AbilitySpec = ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);
	if (!AbilitySpec) return;

	RuntimeSkillDefinition = Cast<UMASkillDefinition>(AbilitySpec->SourceObject.Get());
}
