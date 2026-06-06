#include "GAS/Skill/Step/MASkillStep.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/MAAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Step/MASkillStepManager.h"

void UMASkillStep::StartStep(UMASkillAbility* SkillAbility, EMASkillStepStartMode /*StartMode*/)
{
	OwnerSkillAbility = SkillAbility;
}

void UMASkillStep::ConfigureAssembledStep(
	int32 InStepIndex,
	int32 InNextStepIndex,
	int32 InNextMontageStepIndex,
	int32 InInitialSequenceIndex,
	int32 InSequenceAdvanceCount)
{
	StepIndex = InStepIndex;
	NextStepIndex = InNextStepIndex;
	NextMontageStepIndex = InNextMontageStepIndex;
	SequenceState.AdvanceCount = FMath::Max(InSequenceAdvanceCount, 0);
	SequenceState.MaxIndex = FMath::Max(MaxSequenceSectionCount, 0);
	SequenceState.CurrentIndex = UsesSequenceSections() ? FMath::Max(InInitialSequenceIndex, 1) : 0;
}

void UMASkillStep::HandleStepMontageCancelled()
{
	CurrentMontageTask = nullptr;

	if (UMASkillAbility* SkillAbility = GetOwnerSkillAbility())
	{
		SkillAbility->EndSkill();
	}
}

void UMASkillStep::HandleStepMontageCompleted()
{
	CurrentMontageTask = nullptr;
	if (!ShouldAutoAdvanceOnMontageCompleted()) return;

	RequestAdvanceOrEnd();
}

FName UMASkillStep::ResolveStepStartSectionName() const
{
	return MakeSequenceSectionName(ResolveCurrentSequenceIndex());
}

FString UMASkillStep::GetSequenceSectionKey() const
{
	if (!UsesSequenceSections()) return FString();

	return FString::Printf(
		TEXT("%s|%s"),
		*GetPathNameSafe(ResolveStepMontage()),
		*SequenceSectionNameBase.ToString());
}

bool UMASkillStep::PrepareStepPreview(float PreviewBlendInTime)
{
	constexpr float PreviewPlayRate = 0.01f;

	if (PreparedStepPreviewMontage) return false;

	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAnimInstance* AnimInstance = nullptr;
	UAnimMontage* StepPreviewMontage = nullptr;
	if (!SkillAbility || !SkillAbility->CanPlaySkillMontageLocally() || !TryResolveStepMontageContext(AnimInstance, StepPreviewMontage)) return false;

	if (AnimInstance->Montage_PlayWithBlendSettings(
			StepPreviewMontage,
			FMontageBlendSettings(FMath::Max(PreviewBlendInTime, 0.f)),
			PreviewPlayRate) <= 0.f)
	{
		return false;
	}

	if (const FName StartSectionName = ResolveStepStartSectionName(); !StartSectionName.IsNone())
	{
		AnimInstance->Montage_JumpToSection(StartSectionName, StepPreviewMontage);
	}

	if (UMAAnimInstance* MAAnimInstance = Cast<UMAAnimInstance>(AnimInstance))
	{
		MAAnimInstance->RegisterAnimationOwner(StepPreviewMontage, GetOwnerSkillAbility());
	}
	BindPreparedStepPreviewDelegates(StepPreviewMontage);
	PreparedStepPreviewMontage = StepPreviewMontage;
	bPreparedStepPreviewActivated = false;
	return true;
}

bool UMASkillStep::TryResolveStepMontageContext(UAnimInstance*& OutAnimInstance, UAnimMontage*& OutStepMontage) const
{
	OutAnimInstance = ResolveOwnerAnimInstance();
	OutStepMontage = ResolveStepMontage();
	return OutAnimInstance && OutStepMontage;
}

void UMASkillStep::BroadcastMontageStartedEvent() const
{
	const UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UMASkillModuleInstance* EventScope = SkillAbility ? SkillAbility->GetCurrentSkillModuleInstance() : nullptr;
	if (!EventScope || !BindingScope) return;

	static const FGameplayTag MontageStartedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.MontageStart"));
	FGameplayEventData EventData;
	EventData.EventTag = MontageStartedTag;
	EventData.OptionalObject = BindingScope;
	EventScope->BroadcastScopedEvent(MontageStartedTag, EventData);
}

bool UMASkillStep::PrepareNextStepPreview(float PreviewBlendInTime)
{
	if (ResolveStepMontage()) return false;

	UMASkillStep* NextStep = ResolveNextMontageRuntimeStep();
	return NextStep ? NextStep->PrepareStepPreview(PreviewBlendInTime) : false;
}

bool UMASkillStep::PromotePreparedStepPreviewToActive()
{
	if (!PreparedStepPreviewMontage) return false;

	if (UAnimInstance* AnimInstance = ResolveOwnerAnimInstance())
	{
		if (FAnimMontageInstance* PreparedMontageInstance = AnimInstance->GetInstanceForMontage(PreparedStepPreviewMontage))
		{
			PreparedMontageInstance->SetWeight(1.f);
			PreparedMontageInstance->SetDesiredWeight(1.f);
		}
	}

	bPreparedStepPreviewActivated = true;
	AdvanceSequence();
	return true;
}

void UMASkillStep::ClearPreparedStepPreview(float BlendOutTime)
{
	UAnimInstance* AnimInstance = ResolveOwnerAnimInstance();
	UAnimMontage* StepPreviewMontage = ReleasePreparedStepPreview(false);
	if (!StepPreviewMontage) return;

	if (AnimInstance && AnimInstance->Montage_IsPlaying(StepPreviewMontage))
	{
		AnimInstance->Montage_Stop(BlendOutTime, StepPreviewMontage);
	}
}

int32 UMASkillStep::ResolveCurrentSequenceIndex() const
{
	if (!UsesSequenceSections()) return 0;

	int32 ResolvedSectionIndex = FMath::Max(SequenceState.CurrentIndex, 1);
	if (SequenceState.MaxIndex > 0)
	{
		ResolvedSectionIndex = FMath::Clamp(ResolvedSectionIndex, 1, SequenceState.MaxIndex);
	}

	return ResolvedSectionIndex;
}

void UMASkillStep::AdvanceSequence()
{
	if (!UsesSequenceSections() || SequenceState.AdvanceCount <= 0) return;

	SequenceState.CurrentIndex = FMath::Max(SequenceState.CurrentIndex, 1) + SequenceState.AdvanceCount;
	if (SequenceState.MaxIndex > 0)
	{
		SequenceState.CurrentIndex = ((SequenceState.CurrentIndex - 1) % SequenceState.MaxIndex) + 1;
	}
}

FName UMASkillStep::MakeSequenceSectionName(int32 SectionIndex) const
{
	if (SectionIndex <= 0 || SequenceSectionNameBase.IsNone()) return NAME_None;

	return FName(*FString::Printf(TEXT("%s%d"), *SequenceSectionNameBase.ToString(), SectionIndex));
}

UMASkillStepManager* UMASkillStep::GetOwnerStepManager() const
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	return SkillAbility ? SkillAbility->GetStepManager() : nullptr;
}

void UMASkillStep::RequestAdvanceOrEnd(float MontageBlendOutTime)
{
	if (UMASkillStepManager* StepManager = GetOwnerStepManager())
	{
		StepManager->AdvanceOrEnd(NextStepIndex, MontageBlendOutTime);
	}
}

UAnimInstance* UMASkillStep::ResolveOwnerAnimInstance() const
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	USkeletalMeshComponent* OwningComponent = SkillAbility ? SkillAbility->GetOwningComponentFromActorInfo() : nullptr;
	return OwningComponent ? OwningComponent->GetAnimInstance() : nullptr;
}

UMASkillStep* UMASkillStep::ResolveNextMontageRuntimeStep() const
{
	if (UMASkillStepManager* StepManager = GetOwnerStepManager())
	{
		return StepManager->GetRuntimeSkillStep(NextMontageStepIndex);
	}

	return nullptr;
}

void UMASkillStep::StartCurrentStepMontage()
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAnimInstance* AnimInstance = nullptr;
	UAnimMontage* CurrentStepMontage = nullptr;
	if (!SkillAbility || !SkillAbility->CanPlaySkillMontageLocally() || !TryResolveStepMontageContext(AnimInstance, CurrentStepMontage)) return;

	const FName StartSectionName = ResolveStepStartSectionName();
	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		SkillAbility,
		NAME_None,
		CurrentStepMontage,
		GetOwnerStepManager() ? GetOwnerStepManager()->GetDesiredMontagePlayRate() : 1.f,
		StartSectionName);
	if (!CurrentMontageTask) return;

	CurrentMontageTask->OnBlendOut.AddDynamic(this, &UMASkillStep::HandleCurrentStepMontageCompletedTask);
	CurrentMontageTask->OnCancelled.AddDynamic(this, &UMASkillStep::HandleCurrentStepMontageFailedTask);
	CurrentMontageTask->OnCompleted.AddDynamic(this, &UMASkillStep::HandleCurrentStepMontageCompletedTask);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &UMASkillStep::HandleCurrentStepMontageFailedTask);
	if (UMAAnimInstance* MAAnimInstance = Cast<UMAAnimInstance>(AnimInstance))
	{
		MAAnimInstance->RegisterAnimationOwner(CurrentStepMontage, GetOwnerSkillAbility());
	}
	CurrentMontageTask->ReadyForActivation();
	BroadcastMontageStartedEvent();
	AdvanceSequence();
}

void UMASkillStep::ClearCurrentMontageTask()
{
	if (!CurrentMontageTask) return;

	CurrentMontageTask->OnBlendOut.Clear();
	CurrentMontageTask->OnCancelled.Clear();
	CurrentMontageTask->OnCompleted.Clear();
	CurrentMontageTask->OnInterrupted.Clear();
	CurrentMontageTask->EndTask();
	CurrentMontageTask = nullptr;
}

void UMASkillStep::StopCurrentStepMontage(float MontageBlendOutTime)
{
	UAnimInstance* AnimInstance = nullptr;
	UAnimMontage* CurrentStepMontage = nullptr;
	if (!TryResolveStepMontageContext(AnimInstance, CurrentStepMontage)) return;

	if (UMASkillStepManager* StepManager = GetOwnerStepManager();
		StepManager && StepManager->IsCurrentStepPrepared())
	{
		ReleasePreparedStepPreview(true);
	}

	if (UMAAnimInstance* MAAnimInstance = Cast<UMAAnimInstance>(AnimInstance))
	{
		MAAnimInstance->UnregisterAnimationOwner(CurrentStepMontage, GetOwnerSkillAbility());
	}

	if (AnimInstance->Montage_IsPlaying(CurrentStepMontage))
	{
		AnimInstance->Montage_Stop(MontageBlendOutTime, CurrentStepMontage);
	}
}

UAnimMontage* UMASkillStep::ReleasePreparedStepPreview(bool bKeepAnimationOwnerRegistration)
{
	UAnimMontage* StepPreviewMontage = PreparedStepPreviewMontage;
	if (!StepPreviewMontage) return nullptr;

	ClearPreparedStepPreviewDelegates(StepPreviewMontage);
	if (!bKeepAnimationOwnerRegistration)
	{
		if (UMAAnimInstance* MAAnimInstance = Cast<UMAAnimInstance>(ResolveOwnerAnimInstance()))
		{
			MAAnimInstance->UnregisterAnimationOwner(StepPreviewMontage, GetOwnerSkillAbility());
		}
	}

	PreparedStepPreviewMontage = nullptr;
	bPreparedStepPreviewActivated = false;
	return StepPreviewMontage;
}

void UMASkillStep::FinalizePreparedStepPreview(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage || Montage != PreparedStepPreviewMontage) return;

	const bool bWasPreparedCurrentStep = bPreparedStepPreviewActivated;
	ReleasePreparedStepPreview(bWasPreparedCurrentStep);

	if (bInterrupted)
	{
		HandleStepMontageInterrupted();
		return;
	}

	if (!bWasPreparedCurrentStep)
	{
		if (UMASkillAbility* SkillAbility = GetOwnerSkillAbility())
		{
			SkillAbility->EndSkill();
		}
		return;
	}

	HandleStepMontageCompleted();
}

void UMASkillStep::BindPreparedStepPreviewDelegates(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = ResolveOwnerAnimInstance();
	if (!AnimInstance || !Montage) return;

	FOnMontageBlendingOutStarted BlendingOutDelegate;
	BlendingOutDelegate.BindUObject(this, &UMASkillStep::HandlePreparedStepPreviewBlendingOut);
	AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);

	FOnMontageEnded EndedDelegate;
	EndedDelegate.BindUObject(this, &UMASkillStep::HandlePreparedStepPreviewEnded);
	AnimInstance->Montage_SetEndDelegate(EndedDelegate, Montage);
}

void UMASkillStep::ClearPreparedStepPreviewDelegates(UAnimMontage* Montage)
{
	UAnimInstance* AnimInstance = ResolveOwnerAnimInstance();
	if (!AnimInstance || !Montage) return;

	FOnMontageBlendingOutStarted EmptyBlendingOutDelegate;
	AnimInstance->Montage_SetBlendingOutDelegate(EmptyBlendingOutDelegate, Montage);

	FOnMontageEnded EmptyEndedDelegate;
	AnimInstance->Montage_SetEndDelegate(EmptyEndedDelegate, Montage);
}

void UMASkillStep::HandleCurrentStepMontageCompletedTask()
{
	HandleStepMontageCompleted();
}

void UMASkillStep::HandleCurrentStepMontageFailedTask()
{
	HandleStepMontageCancelled();
}

void UMASkillStep::HandlePreparedStepPreviewBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted) return;
	FinalizePreparedStepPreview(Montage, true);
}

void UMASkillStep::HandlePreparedStepPreviewEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted) return;
	FinalizePreparedStepPreview(Montage, false);
}
