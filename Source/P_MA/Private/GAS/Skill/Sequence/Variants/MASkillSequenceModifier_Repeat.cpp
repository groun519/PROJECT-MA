#include "GAS/Skill/Sequence/Variants/MASkillSequenceModifier_Repeat.h"

void UMASkillSequenceModifier_Repeat::Apply(
	TArray<FMASkillSequence>& Sequences,
	UObject&) const
{
	if (!ensureMsgf(
		Sequences.IsValidIndex(TargetSequenceIndex),
		TEXT("%s has invalid TargetSequenceIndex %d. Sequence count: %d."),
		*GetName(),
		TargetSequenceIndex,
		Sequences.Num()))
	{
		return;
	}
	if (!ensureMsgf(
		AdditionalCopies > 0,
		TEXT("%s requires at least one additional copy."),
		*GetName()))
	{
		return;
	}

	const FMASkillSequence SourceSequence = Sequences[TargetSequenceIndex];
	for (int32 CopyIndex = 0; CopyIndex < AdditionalCopies; ++CopyIndex)
	{
		Sequences.Insert(SourceSequence, TargetSequenceIndex + CopyIndex + 1);
	}
}
