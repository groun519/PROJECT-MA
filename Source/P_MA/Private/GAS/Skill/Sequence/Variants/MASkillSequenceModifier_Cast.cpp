#include "GAS/Skill/Sequence/Variants/MASkillSequenceModifier_Cast.h"

#include "GAS/Skill/Sequence/MASkillSequenceTask.h"

UMASkillSequenceModifier_Cast::UMASkillSequenceModifier_Cast()
{
	ProgressLabel = FText::FromString(TEXT("Cast"));
}

void UMASkillSequenceModifier_Cast::Apply(
	TArray<FMASkillSequence>& Sequences,
	UObject& RuntimeOuter) const
{
	if (!ensureMsgf(CustomMontage, TEXT("%s requires CustomMontage."), *GetName())) return;

	FMASkillSequenceTaskConfig Config;
	Config.TimeLimitSeconds = TimeLimitSeconds;
	Config.bCompleteOnTimeLimit = TimeLimitSeconds > 0.f;
	Config.bShowProgress = bShowProgress;
	Config.ProgressLabel = ProgressLabel;
	Config.bBlockInput = true;
	Config.MontageMode = EMASkillSequenceTaskMontageMode::CustomMontage;
	Config.CustomMontage = CustomMontage;

	AddTask(Sequences, RuntimeOuter, Config);
}
