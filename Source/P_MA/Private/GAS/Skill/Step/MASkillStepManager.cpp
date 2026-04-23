#include "GAS/Skill/Step/MASkillStepManager.h"

#include "GAS/Skill/MASkillAbility.h"

void UMASkillStepManager::Initialize(UMASkillAbility* SkillAbility)
{
	if (!SkillAbility) return;

	OwnerSkillAbility = SkillAbility;
	if (bInitialized) return;

	OwnerSkillAbility->OnSkillActivated().AddUObject(this, &UMASkillStepManager::HandleSkillActivated);
	OwnerSkillAbility->OnSkillDeactivated().AddUObject(this, &UMASkillStepManager::HandleSkillDeactivated);
	bInitialized = true;
}

void UMASkillStepManager::UpdateSteps(const TArray<TObjectPtr<UMASkillStep>>& InRuntimeSteps)
{
	RuntimeSteps = &InRuntimeSteps;

	for (int32 StepIndex = 0; StepIndex < InRuntimeSteps.Num(); ++StepIndex)
	{
		UMASkillStep* RuntimeStep = InRuntimeSteps[StepIndex];
		if (!RuntimeStep) continue;

		const int32 NextStepIndex = InRuntimeSteps.IsValidIndex(StepIndex + 1) ? StepIndex + 1 : INDEX_NONE;
		int32 NextMontageStepIndex = INDEX_NONE;
		for (int32 NextStepCandidateIndex = StepIndex + 1; NextStepCandidateIndex < InRuntimeSteps.Num(); ++NextStepCandidateIndex)
		{
			const UMASkillStep* NextRuntimeStep = InRuntimeSteps[NextStepCandidateIndex];
			if (!NextRuntimeStep || !NextRuntimeStep->ResolveStepMontage()) continue;

			NextMontageStepIndex = NextStepCandidateIndex;
			break;
		}

		RuntimeStep->InitializeStep(OwnerSkillAbility, StepIndex, NextStepIndex, NextMontageStepIndex);
	}
}

void UMASkillStepManager::ResetRuntimeState()
{
	RuntimeSteps = nullptr;
	CurrentStepIndex = INDEX_NONE;
	CurrentStepStartMode = EMASkillStepStartMode::Fresh;
	DesiredMontagePlayRate = 1.f;
}

void UMASkillStepManager::HandleSkillActivated()
{
	DesiredMontagePlayRate = 1.f;
	if (SetActiveStep(0, EMASkillStepStartMode::Fresh))
	{
		if (UMASkillStep* FirstStep = GetCurrentRuntimeSkillStep())
		{
			FirstStep->EnterStep(EMASkillStepStartMode::Fresh);
		}
	}
}

void UMASkillStepManager::HandleSkillDeactivated()
{
	StopActiveStep();
	ClearPreparedStepPreviews();
	CurrentStepIndex = INDEX_NONE;
	CurrentStepStartMode = EMASkillStepStartMode::Fresh;
	DesiredMontagePlayRate = 1.f;
}

bool UMASkillStepManager::SetActiveStep(int32 TargetStepIndex, EMASkillStepStartMode StartMode)
{
	if (!GetRuntimeSkillStep(TargetStepIndex)) return false;

	CurrentStepIndex = TargetStepIndex;
	CurrentStepStartMode = StartMode;
	return true;
}

UMASkillStep* UMASkillStepManager::GetRuntimeSkillStep(int32 StepIndex) const
{
	if (!RuntimeSteps) return nullptr;

	return RuntimeSteps->IsValidIndex(StepIndex)
		? (*RuntimeSteps)[StepIndex]
		: nullptr;
}

bool UMASkillStepManager::TransitionToStep(int32 TargetStepIndex, EMASkillStepStartMode StartMode, float MontageBlendOutTime)
{
	if (!GetRuntimeSkillStep(TargetStepIndex)) return false;

	StopActiveStep(MontageBlendOutTime);
	ClearPreparedStepPreviews();
	SetActiveStep(TargetStepIndex, StartMode);
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->EnterStep(CurrentStepStartMode);
	}
	return true;
}

void UMASkillStepManager::AdvanceOrEnd(int32 NextStepIndex, float MontageBlendOutTime)
{
	if (!GetRuntimeSkillStep(NextStepIndex))
	{
		StopActiveStep(MontageBlendOutTime);
		ClearPreparedStepPreviews();
		CurrentStepIndex = INDEX_NONE;
		CurrentStepStartMode = EMASkillStepStartMode::Fresh;
		
		if (OwnerSkillAbility)
			OwnerSkillAbility->EndSkill();
		
		return;
	}

	TransitionToStep(NextStepIndex, EMASkillStepStartMode::Fresh, MontageBlendOutTime);
}

void UMASkillStepManager::HandleRuntimeEvent(const FGameplayEventData& Payload) const
{
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->HandleRuntimeEvent(Payload);
	}
}

void UMASkillStepManager::SetDesiredMontagePlayRate(float NewPlayRate)
{
	DesiredMontagePlayRate = FMath::Max(NewPlayRate, 0.f);
	ApplyDesiredMontagePlayRate();
}

void UMASkillStepManager::ApplyDesiredMontagePlayRate() const
{
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->ApplyDesiredMontagePlayRate(DesiredMontagePlayRate);
	}
}

bool UMASkillStepManager::GetSkillProgressInfo(FText& OutLabel, float& OutDuration, float& OutRemainingDuration) const
{
	if (const UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		return CurrentStep->GetStepProgressInfo(OutLabel, OutDuration, OutRemainingDuration);
	}

	return false;
}

void UMASkillStepManager::StopActiveStep(float MontageBlendOutTime)
{
	if (UMASkillStep* CurrentStep = GetCurrentRuntimeSkillStep())
	{
		CurrentStep->StopActiveStep(MontageBlendOutTime);
	}
}

void UMASkillStepManager::ClearPreparedStepPreviews() const
{
	if (!RuntimeSteps) return;

	for (UMASkillStep* RuntimeSkillStep : *RuntimeSteps)
	{
		if (!RuntimeSkillStep) continue;
		RuntimeSkillStep->ClearPreparedStepPreview();
	}
}
