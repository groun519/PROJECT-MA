#include "GAS/Skill/Sequence/MASkillSequenceRuntime.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/MAAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Sequence/MASkillSequenceTask.h"

UMASkillAbility& UMASkillSequenceRuntime::GetOwnerAbility() const
{
	return *CastChecked<UMASkillAbility>(GetOuter());
}

const FMASkillSequence* UMASkillSequenceRuntime::GetCurrentSequence() const
{
	return Sequences.IsValidIndex(CurrentSequenceIndex)
		? &Sequences[CurrentSequenceIndex]
		: nullptr;
}

void UMASkillSequenceRuntime::UpdateSequence(const TArray<FMASkillSequence>& InSequences)
{
	Stop();
	Sequences = InSequences;
	SectionIndices.Init(0, Sequences.Num());
}

void UMASkillSequenceRuntime::ResetSequence()
{
	Stop();
	Sequences.Reset();
	SectionIndices.Reset();
}

void UMASkillSequenceRuntime::Start()
{
	if (bRunning) return;

	CurrentSequenceIndex = 0;
	CurrentTaskIndex = 0;
	bRunning = true;
	BindAttackSpeedChanged();
	ExecuteCurrentSequence();
}

void UMASkillSequenceRuntime::Abort()
{
	if (!bRunning) return;

	Stop();
	GetOwnerAbility().K2_CancelAbility();
}

void UMASkillSequenceRuntime::Stop()
{
	if (!bRunning) return;

	UnbindAttackSpeedChanged();
	bRunning = false;
	UMASkillSequenceTask* TaskToAbort = ActiveTask;
	ActiveTask = nullptr;
	if (TaskToAbort)
	{
		TaskToAbort->Abort();
	}

	ClearMontageTask();
	ClearPreparedMontage();
	if (const FMASkillSequence* Sequence = GetCurrentSequence(); Sequence && Sequence->Montage)
	{
		UnregisterAnimationOwner(*Sequence->Montage);
	}
	StopCurrentMontage();
	CurrentSequenceIndex = INDEX_NONE;
	CurrentTaskIndex = INDEX_NONE;
	CurrentSectionName = NAME_None;
	CurrentPlayRate = 1.f;
}

bool UMASkillSequenceRuntime::GetProgressInfo(
	FText& OutLabel,
	float& OutDuration,
	float& OutRemainingDuration) const
{
	return ActiveTask
		&& ActiveTask->GetProgressInfo(OutLabel, OutDuration, OutRemainingDuration);
}

bool UMASkillSequenceRuntime::PrepareCurrentMontage(float PreparationTime)
{
	if (!bRunning) return false;
	if (bMontagePrepared) return true;

	const FMASkillSequence* Sequence = GetCurrentSequence();
	UMASkillAbility& OwnerAbility = GetOwnerAbility();
	UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(
		OwnerAbility.GetAbilitySystemComponentFromActorInfo());
	if (!Sequence || !Sequence->Montage || !ASC || !OwnerAbility.CanPlaySkillMontageLocally()) return false;

	constexpr float PreviewPlayRate = 0.01f;
	if (ASC->PlayMontageWithBlendIn(
			&OwnerAbility,
			OwnerAbility.GetCurrentActivationInfo(),
			Sequence->Montage,
			PreviewPlayRate,
			CurrentSectionName,
			PreparationTime) <= 0.f)
	{
		return false;
	}

	RegisterAnimationOwner(*Sequence->Montage);
	BindPreparedMontageDelegates(*Sequence->Montage);
	bMontagePrepared = true;
	bPreparedMontageActive = false;
	return true;
}

void UMASkillSequenceRuntime::RefreshPlayRate()
{
	if (!bRunning) return;

	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (!Sequence) return;

	float PlayRate = 1.f;
	if (Sequence->bScaleWithAttackSpeed)
	{
		float AttackSpeed = 1.f;
		if (const UAbilitySystemComponent* ASC = GetOwnerAbility().GetAbilitySystemComponentFromActorInfo())
		{
			AttackSpeed = ASC->GetNumericAttribute(UMAAttributeSet::GetAttackSpeedAttribute());
		}
		PlayRate = GetCurrentSectionPlayLength() * (AttackSpeed > 0.f ? AttackSpeed : 1.f);
	}

	const float SkillMultiplier = Sequence->TargetScopes.GetPayloadAccess().Reader.GetScalarProduct(
		UMAAbilitySystemStatics::GetSkillAttackSpeedMultiplierTag());
	CurrentPlayRate = FMath::Max(PlayRate * SkillMultiplier, KINDA_SMALL_NUMBER);

	if (ActiveTask)
	{
		ActiveTask->ApplyPlayRate(CurrentPlayRate);
	}
	if (!bMontagePrepared || bPreparedMontageActive)
	{
		ApplyCurrentMontagePlayRate();
	}
}

const FMASkillScopes* UMASkillSequenceRuntime::GetCurrentTargetScopes() const
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	return Sequence ? &Sequence->TargetScopes : nullptr;
}

void UMASkillSequenceRuntime::NotifyTaskCompletionEvent(
	const FMASkillScopes& SourceScopes,
	const FMASkillSequenceTaskCompletionEvent& CompletionEvent,
	float ProgressRatio)
{
	if (!CompletionEvent.IsValid()) return;

	UMASkillAbility& OwnerAbility = GetOwnerAbility();
	FMASkillEvent Event(CompletionEvent.EventTag, SourceScopes);
	if (CompletionEvent.ProgressRatioPayloadTag.IsValid())
	{
		Event.Payloads.SetScalar(
			CompletionEvent.ProgressRatioPayloadTag,
			FMath::Clamp(ProgressRatio, 0.f, 1.f));
	}
	UMASkillEventRoutingStatics::TryNotifySkillEvent(&OwnerAbility, MoveTemp(Event));
}

void UMASkillSequenceRuntime::ExecuteCurrentSequence()
{
	if (!bRunning) return;

	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (!Sequence)
	{
		Complete();
		return;
	}
	const UMASkillModuleInstance* ModuleScope = Sequence->TargetScopes.Module.Get();
	if (ModuleScope && (!ModuleScope->IsActive() || ModuleScope->IsCooldownActive()))
	{
		CompleteCurrentSequence();
		return;
	}
	if (!Sequence->Montage)
	{
		Abort();
		return;
	}

	CurrentSectionName = ResolveStartSectionName();
	RefreshPlayRate();
	CurrentTaskIndex = 0;
	ExecuteCurrentTask();
}

void UMASkillSequenceRuntime::ExecuteCurrentTask()
{
	if (!bRunning) return;

	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (!Sequence)
	{
		Abort();
		return;
	}
	if (!Sequence->Tasks.IsValidIndex(CurrentTaskIndex))
	{
		PlayCurrentMontage();
		return;
	}

	UMASkillSequenceTask* TaskTemplate = Sequence->Tasks[CurrentTaskIndex];
	ActiveTask = TaskTemplate
		? DuplicateObject<UMASkillSequenceTask>(TaskTemplate, this)
		: nullptr;
	if (!ActiveTask)
	{
		Abort();
		return;
	}

	ActiveTask->Start(
		*this,
		GetOwnerAbility(),
		Sequence->TargetScopes,
		FMASkillSequenceTaskFinishedSignature::CreateUObject(
			this,
			&UMASkillSequenceRuntime::HandleTaskFinished));
}

void UMASkillSequenceRuntime::PlayCurrentMontage()
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	UMASkillAbility& OwnerAbility = GetOwnerAbility();
	if (!Sequence || !Sequence->Montage || !OwnerAbility.CanPlaySkillMontageLocally())
	{
		Abort();
		return;
	}

	if (bMontagePrepared)
	{
		bPreparedMontageActive = true;
		ApplyCurrentMontagePlayRate();
	}
	else
	{
		MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			&OwnerAbility,
			NAME_None,
			Sequence->Montage,
			CurrentPlayRate,
			CurrentSectionName);
		if (!MontageTask)
		{
			Abort();
			return;
		}

		MontageTask->OnBlendOut.AddDynamic(this, &UMASkillSequenceRuntime::HandleMontageCompleted);
		MontageTask->OnCompleted.AddDynamic(this, &UMASkillSequenceRuntime::HandleMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UMASkillSequenceRuntime::HandleMontageAborted);
		MontageTask->OnInterrupted.AddDynamic(this, &UMASkillSequenceRuntime::HandleMontageAborted);
		RegisterAnimationOwner(*Sequence->Montage);
		MontageTask->ReadyForActivation();
		if (!bRunning) return;
	}

	static const FGameplayTag MontageStartedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.MontageStart"));
	UMASkillEventRoutingStatics::TryNotifySkillEvent(&OwnerAbility, FMASkillEvent(
		MontageStartedTag,
		Sequence->TargetScopes));
	AdvanceSectionIndex();
}

void UMASkillSequenceRuntime::CompleteCurrentSequence()
{
	if (!bRunning) return;

	ClearMontageTask();
	ClearPreparedMontage();
	++CurrentSequenceIndex;
	CurrentTaskIndex = 0;
	ExecuteCurrentSequence();
}

void UMASkillSequenceRuntime::HandleTaskFinished(
	UMASkillSequenceTask* FinishedTask,
	bool bAborted)
{
	if (!bRunning || FinishedTask != ActiveTask) return;

	ActiveTask = nullptr;
	if (bAborted)
	{
		Abort();
		return;
	}

	++CurrentTaskIndex;
	ExecuteCurrentTask();
}

void UMASkillSequenceRuntime::ClearMontageTask()
{
	if (!MontageTask) return;

	MontageTask->OnBlendOut.Clear();
	MontageTask->OnCompleted.Clear();
	MontageTask->OnCancelled.Clear();
	MontageTask->OnInterrupted.Clear();
	MontageTask->EndTask();
	MontageTask = nullptr;
}

void UMASkillSequenceRuntime::ClearPreparedMontage()
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (Sequence && Sequence->Montage && bMontagePrepared)
	{
		UAnimInstance* AnimInstance = GetOwnerAbility().GetOwningComponentFromActorInfo()
			? GetOwnerAbility().GetOwningComponentFromActorInfo()->GetAnimInstance()
			: nullptr;
		if (AnimInstance)
		{
			FOnMontageBlendingOutStarted BlendingOutDelegate;
			AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Sequence->Montage);
			FOnMontageEnded EndedDelegate;
			AnimInstance->Montage_SetEndDelegate(EndedDelegate, Sequence->Montage);
		}
		UnregisterAnimationOwner(*Sequence->Montage);
	}

	bMontagePrepared = false;
	bPreparedMontageActive = false;
}

void UMASkillSequenceRuntime::StopCurrentMontage()
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	UMAAbilitySystemComponent* ASC = Cast<UMAAbilitySystemComponent>(
		GetOwnerAbility().GetAbilitySystemComponentFromActorInfo());
	if (Sequence && Sequence->Montage && ASC)
	{
		ASC->StopMontageIfCurrent(*Sequence->Montage, 0.f);
	}
}

void UMASkillSequenceRuntime::RegisterAnimationOwner(UAnimMontage& Montage) const
{
	UMASkillAbility& OwnerAbility = GetOwnerAbility();
	USkeletalMeshComponent* Mesh = OwnerAbility.GetOwningComponentFromActorInfo();
	if (UMAAnimInstance* AnimInstance = Mesh ? Cast<UMAAnimInstance>(Mesh->GetAnimInstance()) : nullptr)
	{
		AnimInstance->RegisterAnimationOwner(&Montage, &OwnerAbility);
	}
}

void UMASkillSequenceRuntime::UnregisterAnimationOwner(UAnimMontage& Montage) const
{
	UMASkillAbility& OwnerAbility = GetOwnerAbility();
	USkeletalMeshComponent* Mesh = OwnerAbility.GetOwningComponentFromActorInfo();
	if (UMAAnimInstance* AnimInstance = Mesh ? Cast<UMAAnimInstance>(Mesh->GetAnimInstance()) : nullptr)
	{
		AnimInstance->UnregisterAnimationOwner(&Montage, &OwnerAbility);
	}
}

void UMASkillSequenceRuntime::BindPreparedMontageDelegates(UAnimMontage& Montage)
{
	USkeletalMeshComponent* Mesh = GetOwnerAbility().GetOwningComponentFromActorInfo();
	UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
	if (!AnimInstance) return;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindUObject(this, &UMASkillSequenceRuntime::HandlePreparedMontageBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, &Montage);

	FOnMontageEnded EndedDelegate;
	EndedDelegate.BindUObject(this, &UMASkillSequenceRuntime::HandlePreparedMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndedDelegate, &Montage);
}

void UMASkillSequenceRuntime::ApplyCurrentMontagePlayRate()
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	UAbilitySystemComponent* ASC = GetOwnerAbility().GetAbilitySystemComponentFromActorInfo();
	if (Sequence && Sequence->Montage && ASC && ASC->GetCurrentMontage() == Sequence->Montage)
	{
		ASC->CurrentMontageSetPlayRate(CurrentPlayRate);
	}
}

float UMASkillSequenceRuntime::GetCurrentSectionPlayLength() const
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (!Sequence || !Sequence->Montage) return 1.f;
	if (CurrentSectionName.IsNone()) return Sequence->Montage->GetPlayLength();

	const int32 SectionIndex = Sequence->Montage->GetSectionIndex(CurrentSectionName);
	return SectionIndex != INDEX_NONE
		? Sequence->Montage->GetSectionLength(SectionIndex)
		: 1.f;
}

FName UMASkillSequenceRuntime::ResolveStartSectionName() const
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (!Sequence || Sequence->SequenceSectionNameBase.IsNone()) return NAME_None;

	int32 SectionIndex = SectionIndices.IsValidIndex(CurrentSequenceIndex)
		? SectionIndices[CurrentSequenceIndex]
		: 0;
	if (SectionIndex <= 0)
	{
		SectionIndex = FMath::Max(Sequence->InitialSequenceIndex, 1);
	}
	if (Sequence->MaxSectionCount > 0)
	{
		SectionIndex = FMath::Clamp(SectionIndex, 1, Sequence->MaxSectionCount);
	}

	return FName(*FString::Printf(
		TEXT("%s%d"),
		*Sequence->SequenceSectionNameBase.ToString(),
		SectionIndex));
}

void UMASkillSequenceRuntime::AdvanceSectionIndex()
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (!Sequence || !SectionIndices.IsValidIndex(CurrentSequenceIndex)
		|| Sequence->SequenceAdvanceCount <= 0)
	{
		return;
	}

	int32& SectionIndex = SectionIndices[CurrentSequenceIndex];
	if (SectionIndex <= 0)
	{
		SectionIndex = FMath::Max(Sequence->InitialSequenceIndex, 1);
	}
	SectionIndex += Sequence->SequenceAdvanceCount;
	if (Sequence->MaxSectionCount > 0)
	{
		SectionIndex = ((SectionIndex - 1) % Sequence->MaxSectionCount) + 1;
	}
}

void UMASkillSequenceRuntime::BindAttackSpeedChanged()
{
	for (const FMASkillSequence& Sequence : Sequences)
	{
		if (!Sequence.bScaleWithAttackSpeed) continue;

		if (UAbilitySystemComponent* ASC = GetOwnerAbility().GetAbilitySystemComponentFromActorInfo())
		{
			AttackSpeedChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(
				UMAAttributeSet::GetAttackSpeedAttribute()).AddUObject(
					this,
					&UMASkillSequenceRuntime::HandleAttackSpeedChanged);
		}
		return;
	}
}

void UMASkillSequenceRuntime::UnbindAttackSpeedChanged()
{
	if (!AttackSpeedChangedHandle.IsValid()) return;

	if (UAbilitySystemComponent* ASC = GetOwnerAbility().GetAbilitySystemComponentFromActorInfo())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(
			UMAAttributeSet::GetAttackSpeedAttribute()).Remove(AttackSpeedChangedHandle);
	}
	AttackSpeedChangedHandle.Reset();
}

void UMASkillSequenceRuntime::HandleAttackSpeedChanged(const FOnAttributeChangeData& /*ChangeData*/)
{
	RefreshPlayRate();
}

void UMASkillSequenceRuntime::HandleMontageCompleted()
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (Sequence && Sequence->Montage)
	{
		UnregisterAnimationOwner(*Sequence->Montage);
	}
	CompleteCurrentSequence();
}

void UMASkillSequenceRuntime::HandleMontageAborted()
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (Sequence && Sequence->Montage)
	{
		UnregisterAnimationOwner(*Sequence->Montage);
	}
	ClearMontageTask();
	Abort();
}

void UMASkillSequenceRuntime::HandlePreparedMontageBlendingOut(
	UAnimMontage* Montage,
	bool bInterrupted)
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (!Sequence || Montage != Sequence->Montage || !bInterrupted) return;

	ClearPreparedMontage();
	Abort();
}

void UMASkillSequenceRuntime::HandlePreparedMontageEnded(
	UAnimMontage* Montage,
	bool bInterrupted)
{
	const FMASkillSequence* Sequence = GetCurrentSequence();
	if (!Sequence || Montage != Sequence->Montage) return;

	const bool bWasActive = bPreparedMontageActive;
	ClearPreparedMontage();
	if (!bInterrupted && bWasActive)
	{
		CompleteCurrentSequence();
	}
	else
	{
		Abort();
	}
}

void UMASkillSequenceRuntime::Complete()
{
	if (!bRunning) return;

	UnbindAttackSpeedChanged();
	bRunning = false;
	ActiveTask = nullptr;
	ClearMontageTask();
	ClearPreparedMontage();
	CurrentSequenceIndex = INDEX_NONE;
	CurrentTaskIndex = INDEX_NONE;
	CurrentSectionName = NAME_None;
	CurrentPlayRate = 1.f;
	GetOwnerAbility().EndSkill();
}
