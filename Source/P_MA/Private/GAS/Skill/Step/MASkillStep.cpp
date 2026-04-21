#include "GAS/Skill/Step/MASkillStep.h"

#include "GAS/Skill/MASkillAbility.h"

namespace
{
int32 ResolveNextMontageStepIndex(const TArray<TObjectPtr<UMASkillStep>>& RuntimeSkillSteps, int32 CurrentIndex)
{
	for (int32 StepIndex = CurrentIndex + 1; StepIndex < RuntimeSkillSteps.Num(); ++StepIndex)
	{
		const UMASkillStep* RuntimeStep = RuntimeSkillSteps[StepIndex];
		if (RuntimeStep && RuntimeStep->ResolveStepMontage())
		{
			return StepIndex;
		}
	}

	return INDEX_NONE;
}
}

void UMASkillStep::StartStep(UMASkillAbility* SkillAbility, EMASkillStepStartMode /*StartMode*/)
{
	OwnerSkillAbility = SkillAbility;
	if (SequenceSectionNameBase.IsNone()) return;

	RuntimeSequenceSectionIndex = ResolveNextSequenceSectionIndex();
}

void UMASkillStep::StopStep()
{
	OwnerSkillAbility = nullptr;
}

void UMASkillStep::CreateRuntimeSteps(UMASkillAbility* SkillAbility,
	const TArray<TObjectPtr<UMASkillStep>>& StepTemplates,
	TArray<TObjectPtr<UMASkillStep>>& OutRuntimeSkillSteps)
{
	OutRuntimeSkillSteps.Reset();
	if (!SkillAbility) return;

	for (UMASkillStep* StepTemplate : StepTemplates)
	{
		if (!StepTemplate) continue;

		UMASkillStep* RuntimeStep = DuplicateObject<UMASkillStep>(StepTemplate, SkillAbility);
		if (!RuntimeStep) continue;

		OutRuntimeSkillSteps.Add(RuntimeStep);
	}

	for (int32 StepIndex = 0; StepIndex < OutRuntimeSkillSteps.Num(); ++StepIndex)
	{
		UMASkillStep* RuntimeStep = OutRuntimeSkillSteps[StepIndex];
		if (!RuntimeStep) continue;

		const int32 NextStepIndex = OutRuntimeSkillSteps.IsValidIndex(StepIndex + 1) ? StepIndex + 1 : INDEX_NONE;
		const int32 NextMontageStepIndex = ResolveNextMontageStepIndex(OutRuntimeSkillSteps, StepIndex);
		RuntimeStep->InitializeStep(SkillAbility, StepIndex, NextStepIndex, NextMontageStepIndex);
	}
}

void UMASkillStep::CollectCurrentRequiredStepEventTags(const TArray<TObjectPtr<UMASkillStep>>& RuntimeSkillSteps,
	int32 CurrentStepIndex, TSet<FGameplayTag>& OutTags)
{
	if (!RuntimeSkillSteps.IsValidIndex(CurrentStepIndex)) return;

	const UMASkillStep* CurrentStep = RuntimeSkillSteps[CurrentStepIndex];
	if (!CurrentStep) return;

	CurrentStep->CollectRequiredEventTags(OutTags);
}

FName UMASkillStep::ResolveStepStartSectionName() const
{
	if (SequenceSectionNameBase.IsNone()) return NAME_None;

	int32 ResolvedSectionIndex = FMath::Max(RuntimeSequenceSectionIndex, 1);
	if (MaxSequenceSectionCount > 0)
		ResolvedSectionIndex = FMath::Clamp(ResolvedSectionIndex, 1, MaxSequenceSectionCount);

	return MakeSequenceSectionName(ResolvedSectionIndex);
}

FName UMASkillStep::ResolvePreparedStepStartSectionName() const
{
	if (SequenceSectionNameBase.IsNone()) return NAME_None;

	return MakeSequenceSectionName(ResolveNextSequenceSectionIndex());
}

int32 UMASkillStep::ResolveNextSequenceSectionIndex() const
{
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
