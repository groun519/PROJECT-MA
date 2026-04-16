#include "GAS/Skill/MASkillAbility.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Character/MACharacter.h"
#include "Character/MAImpulseComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/MASkillEventSource.h"
#include "GAS/Skill/Input/MASkillFlowPart.h"

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

void UMASkillAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!SkillDefinition) { K2_EndAbility(); return; }
	if (!K2_CommitAbility()) { K2_EndAbility(); return; }

	RuntimeContext.Initialize(this);
	DesiredMontagePlayRate = 1.f;
	RegisterFlowParts();
	RegisterEventSources();
	StartCurrentFlow();
	RefreshEventBindings();
	RegisterCancelTriggers();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMASkillAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	UnregisterFlowParts();
	UnregisterEventSources();
	ClearEventTasks();
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
	if (UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart())
	{
		CurrentFlowPart->HandleRuntimeEvent(Payload);
	}

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

	const int32 NextFlowIndex = CurrentFlowPart ? CurrentFlowPart->GetNextMontageFlowIndex() : INDEX_NONE;
	if (!RuntimeFlowParts.IsValidIndex(NextFlowIndex)) return false;

	UMASkillFlowPart* NextFlowPart = RuntimeFlowParts[NextFlowIndex];
	UAnimMontage* NextFlowMontage = NextFlowPart ? NextFlowPart->ResolveFlowMontage() : nullptr;
	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	const bool bCanPlayMontageLocally = ActorInfo
		&& (HasAuthorityOrPredictionKey(ActorInfo, &CurrentActivationInfo) || ActorInfo->IsLocallyControlled());
	if (!bCanPlayMontageLocally || !AnimInstance || !NextFlowMontage) return false;

	if (AnimInstance->Montage_PlayWithBlendSettings(NextFlowMontage, FMontageBlendSettings(FMath::Max(PreviewBlendInTime, 0.f)), KINDA_SMALL_NUMBER) <= 0.f) return false;

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

	InitializeFlowParts();
	CurrentFlowIndex = RuntimeFlowParts.IsEmpty() ? INDEX_NONE : 0;
	CurrentFlowStartMode = EMASkillFlowStartMode::Fresh;
	PreparedFlowIndex = INDEX_NONE;
}

void UMASkillAbility::InitializeFlowParts()
{
	for (int32 FlowIndex = 0; FlowIndex < RuntimeFlowParts.Num(); ++FlowIndex)
	{
		UMASkillFlowPart* RuntimeFlowPart = RuntimeFlowParts[FlowIndex];
		if (!RuntimeFlowPart) continue;

		const int32 NextFlowIndex = RuntimeFlowParts.IsValidIndex(FlowIndex + 1) ? FlowIndex + 1 : INDEX_NONE;
		const int32 NextMontageFlowIndex = ResolveNextMontageFlowIndex(FlowIndex);
		RuntimeFlowPart->InitializeFlow(this, FlowIndex, NextFlowIndex, NextMontageFlowIndex);
	}
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
	const bool bCanPlayMontageLocally = ActorInfo
		&& (HasAuthorityOrPredictionKey(ActorInfo, &CurrentActivationInfo) || ActorInfo->IsLocallyControlled());
	if (!bCanPlayMontageLocally || !FlowMontage) return;

	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, FlowMontage, DesiredMontagePlayRate);
	if (!CurrentMontageTask) return;

	CurrentMontageTask->OnCancelled.AddDynamic(this, &UMASkillAbility::HandleCurrentFlowMontageCancelled);
	CurrentMontageTask->OnCompleted.AddDynamic(this, &UMASkillAbility::HandleCurrentFlowMontageCompleted);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UMASkillAbility::HandleCurrentFlowMontageInterrupted);
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

	const int32 NextFlowIndex = CurrentFlowPart ? CurrentFlowPart->GetNextFlowIndex() : INDEX_NONE;
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

void UMASkillAbility::ClearEventTasks()
{
	for (UAbilityTask_WaitGameplayEvent* Task : EventTasks)
	{
		if (Task) Task->EndTask();
	}
	EventTasks.Reset();
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

void UMASkillAbility::RefreshEventBindings()
{
	ClearEventTasks();
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

	if (const UMASkillFlowPart* CurrentFlowPart = GetCurrentRuntimeFlowPart())
	{
		if (!CurrentFlowPart->ShouldAutoAdvanceOnMontageCompleted()) return;
	}

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
		PreparedMontage = nullptr;
		PreparedFlowIndex = INDEX_NONE;
		K2_EndAbility();
		return;
	}

	HandleCurrentFlowMontageCompleted();
}

int32 UMASkillAbility::ResolveNextMontageFlowIndex(int32 CurrentIndex) const
{
	for (int32 FlowIndex = CurrentIndex + 1; FlowIndex < RuntimeFlowParts.Num(); ++FlowIndex)
	{
		const UMASkillFlowPart* RuntimeFlowPart = RuntimeFlowParts[FlowIndex];
		if (RuntimeFlowPart && RuntimeFlowPart->ResolveFlowMontage())
		{
			return FlowIndex;
		}
	}

	return INDEX_NONE;
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
