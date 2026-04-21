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
#include "GAS/Skill/Step/MASkillStep.h"

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
	CacheRuntimeSkillDefinition(Handle, ActorInfo);
	if (!GetSkillDefinition()) { K2_EndAbility(); return; }
	if (!K2_CommitAbility()) { K2_EndAbility(); return; }

	ResetResolvedData();
	GetSkillDefinition()->ApplyPayloadsTo(PayloadStore);
	GetSkillDefinition()->CollectEventActions(ResolvedRequiredEventTags, ResolvedActionsByEvent);
	RuntimeContext.Initialize(this);
	DesiredMontagePlayRate = 1.f;
	RegisterSkillSteps();
	RegisterEventSources();
	StartCurrentStep();
	RefreshEventBindings();
	RegisterCancelTriggers();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UMASkillAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetStepExecutionState();
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
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->HandleRuntimeEvent(Payload);
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

UMASkillStep* UMASkillAbility::GetCurrentRuntimeSkillStep() const
{
	return RuntimeSkillSteps.IsValidIndex(CurrentStepIndex) ? RuntimeSkillSteps[CurrentStepIndex] : nullptr;
}

void UMASkillAbility::SetDesiredMontagePlayRate(float NewPlayRate)
{
	DesiredMontagePlayRate = FMath::Max(NewPlayRate, 0.f);

	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep();
	UAnimMontage* StepMontage = CurrentStep ? CurrentStep->ResolveStepMontage() : nullptr;
	if (!AnimInstance || !StepMontage) return;

	if (AnimInstance->Montage_IsPlaying(StepMontage))
	{
		AnimInstance->Montage_SetPlayRate(StepMontage, DesiredMontagePlayRate);
	}
}

void UMASkillAbility::CompleteCurrentStep(float MontageBlendOutTime)
{
	if (!AdvanceToNextStep(MontageBlendOutTime))
	{
		K2_EndAbility();
	}
}

bool UMASkillAbility::PrepareNextStepMontage(float PreviewBlendInTime)
{
	if (CurrentStepStartMode != EMASkillStepStartMode::Fresh) return false;
	if (PreparedMontage || PreparedStepIndex != INDEX_NONE) return false;

	UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep();
	if (CurrentStep && CurrentStep->ResolveStepMontage()) return false;

	const int32 NextStepIndex = CurrentStep ? CurrentStep->GetNextMontageStepIndex() : INDEX_NONE;
	if (!RuntimeSkillSteps.IsValidIndex(NextStepIndex)) return false;

	UMASkillStep* NextStep = RuntimeSkillSteps[NextStepIndex];
	UAnimMontage* NextStepMontage = NextStep ? NextStep->ResolveStepMontage() : nullptr;
	const FName StartSectionName = NextStep ? NextStep->ResolvePreparedStepStartSectionName() : NAME_None;
	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	const bool bCanPlayMontageLocally = ActorInfo
		&& (HasAuthorityOrPredictionKey(ActorInfo, &CurrentActivationInfo) || ActorInfo->IsLocallyControlled());
	if (!bCanPlayMontageLocally || !AnimInstance || !NextStepMontage) return false;

	if (AnimInstance->Montage_PlayWithBlendSettings(NextStepMontage, FMontageBlendSettings(FMath::Max(PreviewBlendInTime, 0.f)), KINDA_SMALL_NUMBER) <= 0.f) return false;
	if (!StartSectionName.IsNone())
	{
		AnimInstance->Montage_JumpToSection(StartSectionName, NextStepMontage);
	}

	RegisterAnimationOwner(NextStepMontage);
	BindPreparedMontageDelegates(NextStepMontage);
	PreparedMontage = NextStepMontage;
	PreparedStepIndex = NextStepIndex;
	return true;
}

bool UMASkillAbility::ActivatePreparedNextStep()
{
	if (!RuntimeSkillSteps.IsValidIndex(PreparedStepIndex) || !PreparedMontage) return false;

	UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep();
	if (CurrentStep)
	{
		CurrentStep->StopStep();
	}

	if (UAnimInstance* AnimInstance = GetOwnerAnimInstance())
	{
		if (FAnimMontageInstance* PreparedMontageInstance = AnimInstance->GetInstanceForMontage(PreparedMontage))
		{
			PreparedMontageInstance->SetWeight(1.f);
			PreparedMontageInstance->SetDesiredWeight(1.f);
		}
	}

	CurrentStepIndex = PreparedStepIndex;
	PreparedStepIndex = INDEX_NONE;
	CurrentStepStartMode = EMASkillStepStartMode::Prepared;
	CurrentMontageTask = nullptr;
	PreparedMontage = nullptr;

	StartCurrentStep();
	RefreshEventBindings();
	SetDesiredMontagePlayRate(DesiredMontagePlayRate);
	return true;
}

void UMASkillAbility::RegisterEventSources()
{
	UnregisterEventSources();

	const UMASkillDefinition* ResolvedSkillDefinition = GetSkillDefinition();
	if (!ResolvedSkillDefinition) return;
	UMASkillEventSource::CreateRuntimeSources(this, ResolvedSkillDefinition->GetEventSources(), RuntimeEventSources);
}

void UMASkillAbility::UnregisterEventSources()
{
	UMASkillEventSource::StopRuntimeSources(RuntimeEventSources);
}

void UMASkillAbility::RegisterSkillSteps()
{
	const UMASkillDefinition* ResolvedSkillDefinition = GetSkillDefinition();
	if (!ResolvedSkillDefinition)
	{
		ResetStepExecutionState();
		return;
	}

	if (RuntimeSkillSteps.IsEmpty())
	{
		UMASkillStep::CreateRuntimeSteps(this, ResolvedSkillDefinition->GetSkillSteps(), RuntimeSkillSteps);
	}

	CurrentStepIndex = RuntimeSkillSteps.IsEmpty() ? INDEX_NONE : 0;
	CurrentStepStartMode = EMASkillStepStartMode::Fresh;
	PreparedStepIndex = INDEX_NONE;
}

void UMASkillAbility::UnregisterSkillSteps()
{
	ResetStepExecutionState();
	RuntimeSkillSteps.Reset();
}

void UMASkillAbility::ResetStepExecutionState(float CurrentStepMontageBlendOutTime)
{
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->StopStep();
	}

	ClearCurrentMontageTask();
	StopCurrentStepMontage(CurrentStepMontageBlendOutTime);
	ClearPreparedMontage();
	CurrentStepIndex = INDEX_NONE;
	CurrentStepStartMode = EMASkillStepStartMode::Fresh;
	PreparedStepIndex = INDEX_NONE;
}

void UMASkillAbility::StartCurrentStep()
{
	UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep();
	if (!CurrentStep) return;

	const EMASkillStepStartMode StepStartMode = CurrentStepStartMode;
	CurrentStep->StartStep(this, StepStartMode);
	if (StepStartMode == EMASkillStepStartMode::Prepared) return;

	const FGameplayAbilityActorInfo* ActorInfo = CurrentActorInfo;
	UAnimMontage* StepMontage = CurrentStep->ResolveStepMontage();
	const FName StartSectionName = CurrentStep->ResolveStepStartSectionName();
	const bool bCanPlayMontageLocally = ActorInfo
		&& (HasAuthorityOrPredictionKey(ActorInfo, &CurrentActivationInfo) || ActorInfo->IsLocallyControlled());
	if (!bCanPlayMontageLocally || !StepMontage) return;

	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		StepMontage,
		DesiredMontagePlayRate,
		StartSectionName);
	if (!CurrentMontageTask) return;

	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UMASkillAbility::HandleCurrentStepMontageCompleted);
	CurrentMontageTask->OnCancelled.AddDynamic(this, &UMASkillAbility::HandleCurrentStepMontageCancelled);
	CurrentMontageTask->OnCompleted.AddDynamic(this, &UMASkillAbility::HandleCurrentStepMontageCompleted);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UMASkillAbility::HandleCurrentStepMontageInterrupted);
	RegisterAnimationOwner(StepMontage);
	CurrentMontageTask->ReadyForActivation();
}

bool UMASkillAbility::AdvanceToNextStep(float CurrentStepMontageBlendOutTime)
{
	UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep();
	if (CurrentStep)
	{
		CurrentStep->StopStep();
	}

	ClearCurrentMontageTask();
	StopCurrentStepMontage(CurrentStepMontageBlendOutTime);
	ClearPreparedMontage();

	const int32 NextStepIndex = CurrentStep ? CurrentStep->GetNextStepIndex() : INDEX_NONE;
	if (!RuntimeSkillSteps.IsValidIndex(NextStepIndex))
	{
		CurrentStepIndex = INDEX_NONE;
		return false;
	}

	CurrentStepIndex = NextStepIndex;
	CurrentStepStartMode = EMASkillStepStartMode::Fresh;
	PreparedStepIndex = INDEX_NONE;
	StartCurrentStep();
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

	CurrentMontageTask->OnBlendOut.Clear();
	CurrentMontageTask->OnCancelled.Clear();
	CurrentMontageTask->OnCompleted.Clear();
	CurrentMontageTask->OnInterrupted.Clear();
	CurrentMontageTask->EndTask();
	CurrentMontageTask = nullptr;
}

void UMASkillAbility::StopCurrentStepMontage(float MontageBlendOutTime)
{
	UAnimInstance* AnimInstance = GetOwnerAnimInstance();
	UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep();
	UAnimMontage* StepMontage = CurrentStep ? CurrentStep->ResolveStepMontage() : nullptr;
	if (!AnimInstance || !StepMontage) return;

	UnregisterAnimationOwner(StepMontage);

	if (CurrentStepStartMode == EMASkillStepStartMode::Prepared)
	{
		ClearMontageDelegates(StepMontage);
	}

	if (AnimInstance->Montage_IsPlaying(StepMontage))
	{
		AnimInstance->Montage_Stop(MontageBlendOutTime, StepMontage);
	}
}

void UMASkillAbility::ClearPreparedMontage()
{
	if (!PreparedMontage)
	{
		PreparedStepIndex = INDEX_NONE;
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
	PreparedStepIndex = INDEX_NONE;
}

void UMASkillAbility::RegisterAnimationOwner(UAnimSequenceBase* Animation)
{
	if (!Animation) return;

	if (UMAAnimInstance* AnimInstance = Cast<UMAAnimInstance>(GetOwnerAnimInstance()))
	{
		AnimInstance->RegisterAnimationOwner(Animation, this);
	}
}

void UMASkillAbility::ResetResolvedData()
{
	PayloadStore.Reset();
	ResolvedRequiredEventTags.Reset();
	ResolvedActionsByEvent.Reset();
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
	ClearEventTasks();
	TSet<FGameplayTag> RequiredTags = ResolvedRequiredEventTags;
	UMASkillStep::CollectCurrentRequiredStepEventTags(RuntimeSkillSteps, CurrentStepIndex, RequiredTags);
	for (const FGameplayTag& EventTag : RequiredTags)
	{
		UAbilityTask_WaitGameplayEvent* WaitGameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventTag, nullptr, false, false);
		WaitGameplayEventTask->EventReceived.AddDynamic(this, &UMASkillAbility::HandleSkillGameplayEvent);
		WaitGameplayEventTask->ReadyForActivation();
		EventTasks.Add(WaitGameplayEventTask);
	}
}

void UMASkillAbility::HandleCurrentStepMontageCancelled()
{
	CurrentMontageTask = nullptr;
	K2_EndAbility();
}

void UMASkillAbility::HandleCurrentStepMontageCompleted()
{
	CurrentMontageTask = nullptr;

	if (const UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		if (!CurrentStep->ShouldAutoAdvanceOnMontageCompleted()) return;
	}

	if (!AdvanceToNextStep())
	{
		K2_EndAbility();
	}
}

void UMASkillAbility::HandleCurrentStepMontageInterrupted()
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

	const bool bIsPreparedPreview = Montage == PreparedMontage && PreparedStepIndex != INDEX_NONE;
	const UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep();
	const bool bIsPreparedCurrentStep = CurrentStepStartMode == EMASkillStepStartMode::Prepared
		&& Montage == (CurrentStep ? CurrentStep->ResolveStepMontage() : nullptr);
	if (!bIsPreparedPreview && !bIsPreparedCurrentStep) return;

	ClearMontageDelegates(Montage);
	if (bIsPreparedPreview)
	{
		UnregisterAnimationOwner(Montage);
		PreparedMontage = nullptr;
		PreparedStepIndex = INDEX_NONE;
	}
	HandleCurrentStepMontageInterrupted();
}

void UMASkillAbility::HandlePreparedMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage || bInterrupted) return;

	const bool bIsPreparedPreview = Montage == PreparedMontage && PreparedStepIndex != INDEX_NONE;
	const UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep();
	const bool bIsPreparedCurrentStep = CurrentStepStartMode == EMASkillStepStartMode::Prepared
		&& Montage == (CurrentStep ? CurrentStep->ResolveStepMontage() : nullptr);
	if (!bIsPreparedPreview && !bIsPreparedCurrentStep) return;

	ClearMontageDelegates(Montage);
	if (bIsPreparedPreview)
	{
		UnregisterAnimationOwner(Montage);
		PreparedMontage = nullptr;
		PreparedStepIndex = INDEX_NONE;
		K2_EndAbility();
		return;
	}

	HandleCurrentStepMontageCompleted();
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
