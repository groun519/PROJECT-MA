#include "GAS/Skill/Step/MASkillStep.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/MAAnimInstance.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Step/MASkillStepManager.h"

void UMASkillStep::StartStep(UMASkillAbility* SkillAbility, EMASkillStepStartMode /*StartMode*/)
{
	OwnerSkillAbility = SkillAbility;
	if (!UsesStepSections()) return;

	RuntimeSequenceSectionIndex = ResolveNextSequenceSectionIndex();
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
	UMASkillStepManager* StepManager = GetOwnerStepManager();
	UAnimMontage* PreparedMontage = PreparedStepPreviewMontage;
	if (!SkillAbility || !StepManager || !PreparedMontage) return false;

	if (UMASkillStep* CurrentStep = StepManager->GetCurrentRuntimeSkillStep())
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

	if (!StepManager->TransitionToStep(StepIndex, EMASkillStepStartMode::Prepared, 0.f)) return false;
	StepManager->ApplyDesiredMontagePlayRate();

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

	CurrentMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		SkillAbility,
		NAME_None,
		CurrentStepMontage,
		GetOwnerStepManager() ? GetOwnerStepManager()->GetDesiredMontagePlayRate() : 1.f,
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
