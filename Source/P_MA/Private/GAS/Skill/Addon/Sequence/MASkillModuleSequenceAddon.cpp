#include "GAS/Skill/Addon/Sequence/MASkillModuleSequenceAddon.h"

#include "GAS/Skill/Sequence/MASkillSequenceTask.h"

UMASkillModuleAddon* UMASkillModuleSequenceAddon::AssembleInto(
	UObject& ResultOuter,
	UMASkillModuleAddon* ResultAddon,
	const EMASkillAddonAssemblyStage Stage,
	const FMASkillScopes& SourceScopes) const
{
	if (Sequences.IsEmpty()
		&& (Stage != EMASkillAddonAssemblyStage::ModuleComposition || SequenceModifiers.IsEmpty()))
	{
		return ResultAddon;
	}

	UMASkillModuleSequenceAddon* Result = ResultAddon
		? static_cast<UMASkillModuleSequenceAddon*>(ResultAddon)
		: NewObject<UMASkillModuleSequenceAddon>(&ResultOuter, GetClass());
	for (const FMASkillSequence& SourceSequence : Sequences)
	{
		FMASkillSequence Sequence = SourceSequence;
		Sequence.TargetScopes = Stage == EMASkillAddonAssemblyStage::SkillAssembly
			? SourceScopes
			: FMASkillScopes();
		Sequence.Tasks.Reset(Stage == EMASkillAddonAssemblyStage::SkillAssembly
			? SourceSequence.Tasks.Num()
			: 0);
		Sequence.InitialSequenceIndex = 0;
		Sequence.SequenceAdvanceCount = 0;

		if (Stage == EMASkillAddonAssemblyStage::SkillAssembly)
		{
			for (const UMASkillSequenceTask* SourceTask : SourceSequence.Tasks)
			{
				if (UMASkillSequenceTask* Task = SourceTask
					? DuplicateObject<UMASkillSequenceTask>(SourceTask, Result)
					: nullptr)
				{
					Sequence.Tasks.Add(Task);
				}
			}
		}
		Result->Sequences.Add(MoveTemp(Sequence));
	}

	if (Stage == EMASkillAddonAssemblyStage::ModuleComposition)
	{
		Result->SequenceModifiers.Append(SequenceModifiers);
	}
	return Result;
}

bool UMASkillModuleSequenceAddon::Finalize(const EMASkillAddonAssemblyStage Stage)
{
	if (Stage == EMASkillAddonAssemblyStage::ModuleComposition)
	{
		for (const UMASkillSequenceModifier* Modifier : SequenceModifiers)
		{
			if (Modifier) Modifier->Apply(Sequences, *this);
		}
		SequenceModifiers.Reset();
		return !Sequences.IsEmpty();
	}
	if (Sequences.IsEmpty()) return false;

	TMap<FString, int32> SequenceCounts;
	for (const FMASkillSequence& Sequence : Sequences)
	{
		if (Sequence.UsesSequenceSections())
		{
			SequenceCounts.FindOrAdd(Sequence.GetSequenceKey())++;
		}
	}

	TMap<FString, int32> SequenceOffsets;
	for (FMASkillSequence& Sequence : Sequences)
	{
		if (!Sequence.UsesSequenceSections()) continue;

		const FString SequenceKey = Sequence.GetSequenceKey();
		const int32 InitialSequenceIndex = SequenceOffsets.FindRef(SequenceKey) + 1;
		SequenceOffsets.Add(SequenceKey, InitialSequenceIndex);
		Sequence.InitialSequenceIndex = InitialSequenceIndex;
		Sequence.SequenceAdvanceCount = SequenceCounts.FindRef(SequenceKey);
	}
	return true;
}
