#include "GAS/Skill/Definition/Assembly/MASkillFeatureAssemblers.h"

#include "GAS/Skill/Addon/Cooldown/MASkillCooldownAddon.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventBindingAddon.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventSourceAddon.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"

void FMASkillCooldownAssembler::AppendFrom(
	UMASkillDefinition& TargetDefinition,
	const UMASkillDefinition& SourceDefinition)
{
	const float CooldownSeconds = SourceDefinition.GetCooldownSeconds();
	if (CooldownSeconds <= 0.f) return;

	UMASkillCooldownAddon* TargetAddon = nullptr;
	for (UMASkillModuleAddon* Addon : TargetDefinition.Addons)
	{
		TargetAddon = Cast<UMASkillCooldownAddon>(Addon);
		if (TargetAddon) break;
	}

	if (!TargetAddon)
	{
		TargetAddon = NewObject<UMASkillCooldownAddon>(&TargetDefinition);
		TargetDefinition.Addons.Add(TargetAddon);
	}

	TargetAddon->CooldownSeconds += CooldownSeconds;
}

void FMASkillPayloadAssembler::AppendFrom(
	UMASkillDefinition& TargetDefinition,
	const UMASkillDefinition& SourceDefinition)
{
	TargetDefinition.Payloads.Append(SourceDefinition.Payloads);
}

void FMASkillEventSourceAssembler::AppendFrom(
	UMASkillDefinition& TargetDefinition,
	const UMASkillDefinition& SourceDefinition)
{
	const UMASkillModuleEventSourceAddon* SourceAddon =
		SourceDefinition.FindAddon<UMASkillModuleEventSourceAddon>();
	if (!SourceAddon) return;

	UMASkillModuleEventSourceAddon* TargetAddon = nullptr;
	for (UMASkillModuleAddon* Addon : TargetDefinition.Addons)
	{
		TargetAddon = Cast<UMASkillModuleEventSourceAddon>(Addon);
		if (TargetAddon) break;
	}

	if (!TargetAddon)
	{
		TargetAddon = NewObject<UMASkillModuleEventSourceAddon>(&TargetDefinition);
		TargetDefinition.Addons.Add(TargetAddon);
	}

	for (UMASkillEventSource* EventSource : SourceAddon->EventSources)
	{
		if (!EventSource) continue;

		UMASkillEventSource* NewEventSource = DuplicateObject<UMASkillEventSource>(EventSource, TargetAddon);
		if (NewEventSource) TargetAddon->EventSources.Add(NewEventSource);
	}
}

void FMASkillEventBindingAssembler::AppendFrom(
	UMASkillDefinition& TargetDefinition,
	const UMASkillDefinition& SourceDefinition,
	UMASkillModuleInstance& SourceModuleInstance,
	UMASkillModuleInstance& AssembledModuleInstance)
{
	const UMASkillModuleEventBindingAddon* SourceAddon =
		SourceDefinition.FindAddon<UMASkillModuleEventBindingAddon>();
	if (!SourceAddon) return;

	UMASkillModuleEventBindingAddon* TargetAddon = nullptr;
	for (UMASkillModuleAddon* Addon : TargetDefinition.Addons)
	{
		TargetAddon = Cast<UMASkillModuleEventBindingAddon>(Addon);
		if (TargetAddon) break;
	}

	if (!TargetAddon)
	{
		TargetAddon = NewObject<UMASkillModuleEventBindingAddon>(&TargetDefinition);
		TargetDefinition.Addons.Add(TargetAddon);
	}

	for (const FMASkillEventBinding& EventBinding : SourceAddon->EventBindings)
	{
		FMASkillEventBinding NewEventBinding = EventBinding;
		NewEventBinding.BindingScopes.Module = &SourceModuleInstance;
		NewEventBinding.BindingScopes.Skill = &AssembledModuleInstance;
		TargetAddon->EventBindings.Add(MoveTemp(NewEventBinding));
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
