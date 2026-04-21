#include "GAS/Skill/Step/MASkillStep.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/MAAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/MASkillAbility.h"

void UMASkillStep::StartStep(UMASkillAbility* SkillAbility, EMASkillStepStartMode /*StartMode*/)
{
	OwnerSkillAbility = SkillAbility;
	if (!UsesStepSections()) return;

	RuntimeSequenceSectionIndex = ResolveNextSequenceSectionIndex();
}

void UMASkillStep::HandleStepMontageCancelled()
{
	CurrentMontageTask = nullptr;

	if (UMASkillDefinition* SkillDefinition = GetOwnerSkillDefinition())
	{
		SkillDefinition->EndOwningSkillAbility();
	}
}

void UMASkillStep::HandleStepMontageCompleted()
{
	CurrentMontageTask = nullptr;
	if (!ShouldAutoAdvanceOnMontageCompleted()) return;

	CompleteOrEndOwnerStep();
}

FName UMASkillStep::ResolveStepStartSectionName() const
{
	return MakeSequenceSectionName(ResolveCurrentSequenceSectionIndex());
}

FName UMASkillStep::ResolvePreparedStepStartSectionName() const
{
	return MakeSequenceSectionName(ResolveNextSequenceSectionIndex());
}

bool UMASkillStep::PrepareStepPreview(float PreviewBlendInTime)
{
	if (PreparedStepPreviewMontage) return false;

	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAnimInstance* AnimInstance = nullptr;
	UAnimMontage* StepPreviewMontage = nullptr;
	if (!SkillAbility || !SkillAbility->CanPlaySkillMontageLocally() || !TryResolveStepMontageContext(AnimInstance, StepPreviewMontage)) return false;

	if (AnimInstance->Montage_PlayWithBlendSettings(
			StepPreviewMontage,
			FMontageBlendSettings(FMath::Max(PreviewBlendInTime, 0.f)),
			KINDA_SMALL_NUMBER) <= 0.f)
	{
		return false;
	}

	if (const FName StartSectionName = ResolvePreparedStepStartSectionName(); !StartSectionName.IsNone())
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

bool UMASkillStep::PrepareNextStepPreview(float PreviewBlendInTime)
{
	if (ResolveStepMontage()) return false;

	UMASkillStep* NextStep = ResolveNextMontageRuntimeStep();
	return NextStep ? NextStep->PrepareStepPreview(PreviewBlendInTime) : false;
}

bool UMASkillStep::ActivatePreparedStepPreview()
{
	if (!PreparedStepPreviewMontage) return false;

	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UMASkillDefinition* SkillDefinition = GetOwnerSkillDefinition();
	UAnimMontage* PreparedMontage = PreparedStepPreviewMontage;
	if (!SkillAbility || !SkillDefinition || !PreparedMontage) return false;

	if (UMASkillStep* CurrentStep = SkillDefinition->GetCurrentRuntimeSkillStep())
	{
		CurrentStep->StopStep();
	}

	if (UAnimInstance* AnimInstance = ResolveOwnerAnimInstance())
	{
		if (FAnimMontageInstance* PreparedMontageInstance = AnimInstance->GetInstanceForMontage(PreparedMontage))
		{
			PreparedMontageInstance->SetWeight(1.f);
			PreparedMontageInstance->SetDesiredWeight(1.f);
		}
	}

	if (!TransitionOwnerDefinitionStep(StepIndex, EMASkillStepStartMode::Prepared, 0.f)) return false;
	SkillAbility->SetDesiredMontagePlayRate(SkillAbility->GetDesiredMontagePlayRate());

	bPreparedStepPreviewActivated = true;
	return true;
}

bool UMASkillStep::ActivatePreparedNextStepPreview()
{
	UMASkillStep* NextStep = ResolveNextMontageRuntimeStep();
	return NextStep ? NextStep->ActivatePreparedStepPreview() : false;
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

int32 UMASkillStep::ResolveCurrentSequenceSectionIndex() const
{
	if (!UsesStepSections()) return 0;

	int32 ResolvedSectionIndex = FMath::Max(RuntimeSequenceSectionIndex, 1);
	if (MaxSequenceSectionCount > 0)
	{
		ResolvedSectionIndex = FMath::Clamp(ResolvedSectionIndex, 1, MaxSequenceSectionCount);
	}

	return ResolvedSectionIndex;
}

int32 UMASkillStep::ResolveNextSequenceSectionIndex() const
{
	if (!UsesStepSections()) return 0;

	if (MaxSequenceSectionCount > 0)
	{
		return (RuntimeSequenceSectionIndex % MaxSequenceSectionCount) + 1;
	}

	return FMath::Max(RuntimeSequenceSectionIndex + 1, 1);
}

FName UMASkillStep::MakeSequenceSectionName(int32 SectionIndex) const
{
	if (SectionIndex <= 0 || SequenceSectionNameBase.IsNone()) return NAME_None;

	return FName(*FString::Printf(TEXT("%s%d"), *SequenceSectionNameBase.ToString(), SectionIndex));
}

UMASkillDefinition* UMASkillStep::GetOwnerSkillDefinition() const
{
	return GetTypedOuter<UMASkillDefinition>();
}

void UMASkillStep::CompleteOrEndOwnerStep(float MontageBlendOutTime)
{
	UMASkillDefinition* SkillDefinition = GetOwnerSkillDefinition();
	if (!SkillDefinition) return;

	const int32 NextRuntimeStepIndex = NextStepIndex;
	if (!SkillDefinition->GetRuntimeSkillStep(NextRuntimeStepIndex))
	{
		StopCurrentOwnerDefinitionStep(MontageBlendOutTime);
		SkillDefinition->CurrentStepIndex = INDEX_NONE;
		SkillDefinition->EndOwningSkillAbility();
		return;
	}

	TransitionOwnerDefinitionStep(NextRuntimeStepIndex, EMASkillStepStartMode::Fresh, MontageBlendOutTime);
}

UAnimInstance* UMASkillStep::ResolveOwnerAnimInstance() const
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	USkeletalMeshComponent* OwningComponent = SkillAbility ? SkillAbility->GetOwningComponentFromActorInfo() : nullptr;
	return OwningComponent ? OwningComponent->GetAnimInstance() : nullptr;
}

UMASkillStep* UMASkillStep::ResolveNextMontageRuntimeStep() const
{
	UMASkillDefinition* SkillDefinition = GetOwnerSkillDefinition();
	return SkillDefinition ? SkillDefinition->GetRuntimeSkillStep(NextMontageStepIndex) : nullptr;
}

bool UMASkillStep::TransitionOwnerDefinitionStep(int32 TargetStepIndex, EMASkillStepStartMode StartMode, float MontageBlendOutTime)
{
	UMASkillDefinition* SkillDefinition = GetOwnerSkillDefinition();
	if (!SkillDefinition || !SkillDefinition->GetRuntimeSkillStep(TargetStepIndex)) return false;

	StopCurrentOwnerDefinitionStep(MontageBlendOutTime);
	SkillDefinition->CurrentStepIndex = TargetStepIndex;
	SkillDefinition->CurrentStepStartMode = StartMode;
	SkillDefinition->EnterCurrentStep();
	SkillDefinition->RebindEventTasks();
	return true;
}

void UMASkillStep::StopCurrentOwnerDefinitionStep(float MontageBlendOutTime)
{
	if (UMASkillDefinition* SkillDefinition = GetOwnerSkillDefinition())
	{
		if (UMASkillStep* CurrentStep = SkillDefinition->GetCurrentRuntimeSkillStep())
		{
			CurrentStep->StopActiveStep(MontageBlendOutTime);
		}

		SkillDefinition->ClearPreparedStepPreviews();
	}
}

void UMASkillStep::StartCurrentStepMontage()
{
	UMASkillAbility* SkillAbility = GetOwnerSkillAbility();
	UAnimInstance* AnimInstance = nullptr;
	UAnimMontage* CurrentStepMontage = nullptr;
	if (!SkillAbility || !SkillAbility->CanPlaySkillMontageLocally() || !TryResolveStepMontageContext(AnimInstance, CurrentStepMontage)) return;

	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		SkillAbility,
		NAME_None,
		CurrentStepMontage,
		SkillAbility->GetDesiredMontagePlayRate(),
		ResolveStepStartSectionName());
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

	if (UMASkillDefinition* SkillDefinition = GetOwnerSkillDefinition();
		SkillDefinition && SkillDefinition->CurrentStepStartMode == EMASkillStepStartMode::Prepared)
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

	UMASkillDefinition* SkillDefinition = GetOwnerSkillDefinition();
	const bool bWasPreparedCurrentStep = bPreparedStepPreviewActivated;
	ReleasePreparedStepPreview(bWasPreparedCurrentStep);

	if (bInterrupted)
	{
		HandleStepMontageInterrupted();
		return;
	}

	if (!bWasPreparedCurrentStep)
	{
		if (SkillDefinition) SkillDefinition->EndOwningSkillAbility();
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
