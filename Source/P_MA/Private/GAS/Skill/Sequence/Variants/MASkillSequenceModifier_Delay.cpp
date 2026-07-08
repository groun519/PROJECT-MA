#include "GAS/Skill/Sequence/Variants/MASkillSequenceModifier_Delay.h"

#include "GAS/Skill/Sequence/MASkillSequenceTask.h"

UMASkillSequenceModifier_Delay::UMASkillSequenceModifier_Delay()
{
	ProgressLabel = FText::FromString(TEXT("Delay"));
}

void UMASkillSequenceModifier_Delay::Apply(
	TArray<FMASkillSequence>& Sequences,
	UObject& RuntimeOuter) const
{
	FMASkillSequenceTaskConfig Config;
	Config.TimeLimitSeconds = TimeLimitSeconds;
	Config.bCompleteOnTimeLimit = true;
	Config.bShowProgress = bShowProgress;
	Config.ProgressLabel = ProgressLabel;

	AddTask(Sequences, RuntimeOuter, Config);
}
