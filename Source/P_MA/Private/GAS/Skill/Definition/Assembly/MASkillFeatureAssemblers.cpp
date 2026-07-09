#include "GAS/Skill/Definition/Assembly/MASkillFeatureAssemblers.h"

#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"

void FMASkillCooldownAssembler::AppendFrom(
	UMASkillDefinition& TargetDefinition,
	const UMASkillDefinition& SourceDefinition)
{
	TargetDefinition.CooldownSeconds += SourceDefinition.CooldownSeconds;
}

void FMASkillPayloadAssembler::AppendFrom(
	UMASkillDefinition& TargetDefinition,
	const UMASkillDefinition& SourceDefinition)
{
	TargetDefinition.Payloads.Append(SourceDefinition.Payloads);
}

void FMASkillEventAssembler::AppendFrom(
	UMASkillDefinition& TargetDefinition,
	const UMASkillDefinition& SourceDefinition,
	UMASkillModuleInstance& SourceModuleInstance,
	UMASkillModuleInstance& AssembledModuleInstance)
{
	for (UMASkillEventSource* EventSource : SourceDefinition.EventSources)
	{
		if (!EventSource) continue;

		UMASkillEventSource* NewEventSource = DuplicateObject<UMASkillEventSource>(EventSource, &TargetDefinition);
		if (!NewEventSource) continue;

		TargetDefinition.EventSources.Add(NewEventSource);
	}

	for (const FMASkillEventBinding& EventBinding : SourceDefinition.EventBindings)
	{
		FMASkillEventBinding NewEventBinding = EventBinding;
		NewEventBinding.BindingScopes.Module = &SourceModuleInstance;
		NewEventBinding.BindingScopes.Skill = &AssembledModuleInstance;
		NewEventBinding.Action = EventBinding.Action
			? DuplicateObject<UMASkillAction>(EventBinding.Action, &TargetDefinition)
			: nullptr;
		TargetDefinition.EventBindings.Add(MoveTemp(NewEventBinding));
	}
}

void FMASkillSequenceAssembler::AppendFrom(
	UMASkillDefinition& TargetDefinition,
	const UMASkillDefinition& SourceDefinition,
	const FMASkillScopes& TargetScopes)
{
	TArray<FMASkillSequence> ModuleSequences;
	TArray<const UMASkillSequenceModifier*> SequenceModifiers;

	for (const FMASkillSequence& SourceSequence : SourceDefinition.GetBaseSequences())
	{
		FMASkillSequence Sequence = SourceSequence;
		Sequence.TargetScopes = TargetScopes;
		Sequence.Tasks.Reset();
		Sequence.InitialSequenceIndex = 0;
		Sequence.SequenceAdvanceCount = 0;
		ModuleSequences.Add(MoveTemp(Sequence));
	}

	for (const UMASkillSequenceModifier* Modifier : SourceDefinition.GetSequenceModifiers())
	{
		if (Modifier) SequenceModifiers.Add(Modifier);
	}

	for (const UMASkillSequenceModifier* Modifier : SequenceModifiers)
	{
		Modifier->Apply(ModuleSequences, TargetDefinition);
	}

	TargetDefinition.AssembledSequences.Append(MoveTemp(ModuleSequences));
}

void FMASkillSequenceAssembler::Finalize(UMASkillDefinition& TargetDefinition)
{
	TMap<FString, int32> SequenceCounts;
	for (const FMASkillSequence& Sequence : TargetDefinition.AssembledSequences)
	{
		if (Sequence.UsesSequenceSections())
		{
			SequenceCounts.FindOrAdd(Sequence.GetSequenceKey())++;
		}
	}

	TMap<FString, int32> SequenceOffsets;
	for (FMASkillSequence& Sequence : TargetDefinition.AssembledSequences)
	{
		if (!Sequence.UsesSequenceSections()) continue;

		const FString SequenceKey = Sequence.GetSequenceKey();
		const int32 InitialSequenceIndex = SequenceOffsets.FindRef(SequenceKey) + 1;
		SequenceOffsets.Add(SequenceKey, InitialSequenceIndex);
		Sequence.InitialSequenceIndex = InitialSequenceIndex;
		Sequence.SequenceAdvanceCount = SequenceCounts.FindRef(SequenceKey);
	}
}
