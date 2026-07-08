#include "GAS/Skill/Sequence/Variants/MASkillSequenceModifier_Windup.h"

#include "GAS/Skill/Sequence/MASkillSequenceTask.h"

UMASkillSequenceModifier_Windup::UMASkillSequenceModifier_Windup()
{
	ProgressLabel = FText::FromString(TEXT("Windup"));
}

void UMASkillSequenceModifier_Windup::Configure(float InDuration)
{
	TimeLimitSeconds = FMath::Max(InDuration, 0.f);
}

void UMASkillSequenceModifier_Windup::Apply(
	TArray<FMASkillSequence>& Sequences,
	UObject& RuntimeOuter) const
{
	if (TimeLimitSeconds <= 0.f) return;

	FMASkillSequenceTaskConfig Config;
	Config.TimeLimitSeconds = TimeLimitSeconds;
	Config.bCompleteOnTimeLimit = true;
	Config.bShowProgress = bShowProgress;
	Config.ProgressLabel = ProgressLabel;
	Config.MontageMode = EMASkillSequenceTaskMontageMode::PrepareCurrentMontage;

	AddTask(Sequences, RuntimeOuter, Config);
}
