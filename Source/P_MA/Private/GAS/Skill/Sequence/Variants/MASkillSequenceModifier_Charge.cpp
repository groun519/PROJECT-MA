#include "GAS/Skill/Sequence/Variants/MASkillSequenceModifier_Charge.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Sequence/MASkillSequenceTask.h"

UMASkillSequenceModifier_Charge::UMASkillSequenceModifier_Charge()
{
	ProgressLabel = FText::FromString(TEXT("Charge"));
}

void UMASkillSequenceModifier_Charge::Apply(
	TArray<FMASkillSequence>& Sequences,
	UObject& RuntimeOuter) const
{
	FMASkillSequenceTaskConfig Config;
	Config.TimeLimitSeconds = TimeLimitSeconds;
	Config.bCompleteOnTimeLimit = true;
	Config.bShowProgress = bShowProgress;
	Config.ProgressLabel = ProgressLabel;
	Config.MontageMode = EMASkillSequenceTaskMontageMode::PrepareCurrentMontage;
	Config.bWaitInputRelease = true;
	Config.CompletionEvent.EventTag = UMAAbilitySystemStatics::GetChargeCompletedEventTag();
	Config.CompletionEvent.ProgressRatioPayloadTag = UMAAbilitySystemStatics::GetSkillChargeRatioTag();

	AddTask(Sequences, RuntimeOuter, Config);
}
