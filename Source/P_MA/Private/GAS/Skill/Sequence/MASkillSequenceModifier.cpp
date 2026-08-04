#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"

#include "GAS/Skill/Sequence/MASkillSequenceTask.h"

UMASkillSequenceTask* UMASkillSequenceModifier::AddTask(
	TArray<FMASkillSequence>& Sequences,
	UObject& RuntimeOuter,
	const FMASkillSequenceTaskConfig& Config) const
{
	if (!ensureMsgf(
		Sequences.IsValidIndex(TargetSequenceIndex),
		TEXT("%s has invalid TargetSequenceIndex %d. Sequence count: %d."),
		*GetName(),
		TargetSequenceIndex,
		Sequences.Num()))
	{
		return nullptr;
	}

	UMASkillSequenceTask* Task = NewObject<UMASkillSequenceTask>(&RuntimeOuter);
	if (Task)
	{
		Task->Configure(Config);
		Sequences[TargetSequenceIndex].Tasks.Add(Task);
	}
	return Task;
}
