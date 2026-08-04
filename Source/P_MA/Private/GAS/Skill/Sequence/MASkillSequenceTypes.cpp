#include "GAS/Skill/Sequence/MASkillSequenceTypes.h"

FString FMASkillSequence::GetSequenceKey() const
{
	if (!UsesSequenceSections()) return FString();

	return FString::Printf(
		TEXT("%s|%s"),
		*GetPathNameSafe(Montage),
		*SequenceSectionNameBase.ToString());
}
