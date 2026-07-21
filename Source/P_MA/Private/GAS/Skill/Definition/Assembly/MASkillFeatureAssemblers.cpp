#include "GAS/Skill/Definition/Assembly/MASkillFeatureAssemblers.h"

#include "GAS/Skill/Addon/Cooldown/MASkillCooldownAddon.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventBindingAddon.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventSourceAddon.h"
#include "GAS/Skill/Addon/Sequence/MASkillModuleSequenceAddon.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Sequence/MASkillSequenceModifier.h"

void FMASkillCooldownAssembler::AppendFrom(
	UMASkillModule& TargetModule,
	const UMASkillModule& SourceModule)
{
	const float CooldownSeconds = SourceModule.GetCooldownSeconds();
	if (CooldownSeconds <= 0.f) return;

	UMASkillCooldownAddon* TargetAddon = nullptr;
	for (UMASkillModuleAddon* Addon : TargetModule.ModuleData.Addons)
	{
		TargetAddon = Cast<UMASkillCooldownAddon>(Addon);
		if (TargetAddon) break;
	}

	if (!TargetAddon)
	{
		TargetAddon = NewObject<UMASkillCooldownAddon>(&TargetModule);
		TargetModule.ModuleData.Addons.Add(TargetAddon);
	}

	TargetAddon->CooldownSeconds += CooldownSeconds;
}

void FMASkillPayloadAssembler::AppendFrom(
	UMASkillModule& TargetModule,
	const UMASkillModule& SourceModule)
{
	TargetModule.ModuleData.Payloads.Append(SourceModule.ModuleData.Payloads);
}

void FMASkillEventSourceAssembler::AppendFrom(
	UMASkillModule& TargetModule,
	const UMASkillModule& SourceModule)
{
	const UMASkillModuleEventSourceAddon* SourceAddon =
		SourceModule.FindAddon<UMASkillModuleEventSourceAddon>();
	if (!SourceAddon) return;

	UMASkillModuleEventSourceAddon* TargetAddon = nullptr;
	for (UMASkillModuleAddon* Addon : TargetModule.ModuleData.Addons)
	{
		TargetAddon = Cast<UMASkillModuleEventSourceAddon>(Addon);
		if (TargetAddon) break;
	}

	if (!TargetAddon)
	{
		TargetAddon = NewObject<UMASkillModuleEventSourceAddon>(&TargetModule);
		TargetModule.ModuleData.Addons.Add(TargetAddon);
	}

	for (UMASkillEventSource* EventSource : SourceAddon->EventSources)
	{
		if (!EventSource) continue;

		UMASkillEventSource* NewEventSource = DuplicateObject<UMASkillEventSource>(EventSource, TargetAddon);
		if (NewEventSource) TargetAddon->EventSources.Add(NewEventSource);
	}
}

void FMASkillEventBindingAssembler::AppendFrom(
	UMASkillModule& TargetModule,
	const UMASkillModule& SourceModule,
	UMASkillModuleInstance& SourceModuleInstance,
	UMASkillModuleInstance& AssembledModuleInstance)
{
	const UMASkillModuleEventBindingAddon* SourceAddon =
		SourceModule.FindAddon<UMASkillModuleEventBindingAddon>();
	if (!SourceAddon) return;

	UMASkillModuleEventBindingAddon* TargetAddon = nullptr;
	for (UMASkillModuleAddon* Addon : TargetModule.ModuleData.Addons)
	{
		TargetAddon = Cast<UMASkillModuleEventBindingAddon>(Addon);
		if (TargetAddon) break;
	}

	if (!TargetAddon)
	{
		TargetAddon = NewObject<UMASkillModuleEventBindingAddon>(&TargetModule);
		TargetModule.ModuleData.Addons.Add(TargetAddon);
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
	UMASkillModule& TargetModule,
	const UMASkillModule& SourceModule,
	const FMASkillScopes& TargetScopes)
{
	const UMASkillModuleSequenceAddon* SourceAddon =
		SourceModule.FindAddon<UMASkillModuleSequenceAddon>();
	if (!SourceAddon || SourceAddon->Sequences.IsEmpty()) return;

	UMASkillModuleSequenceAddon* TargetAddon = nullptr;
	for (UMASkillModuleAddon* Addon : TargetModule.ModuleData.Addons)
	{
		TargetAddon = Cast<UMASkillModuleSequenceAddon>(Addon);
		if (TargetAddon) break;
	}

	if (!TargetAddon)
	{
		TargetAddon = NewObject<UMASkillModuleSequenceAddon>(&TargetModule);
		TargetModule.ModuleData.Addons.Add(TargetAddon);
	}

	TArray<FMASkillSequence> ModuleSequences;

	for (const FMASkillSequence& SourceSequence : SourceAddon->Sequences)
	{
		FMASkillSequence Sequence = SourceSequence;
		Sequence.TargetScopes = TargetScopes;
		Sequence.Tasks.Reset();
		Sequence.InitialSequenceIndex = 0;
		Sequence.SequenceAdvanceCount = 0;
		ModuleSequences.Add(MoveTemp(Sequence));
	}

	for (const UMASkillSequenceModifier* Modifier : SourceAddon->SequenceModifiers)
	{
		if (Modifier) Modifier->Apply(ModuleSequences, *TargetAddon);
	}

	TargetAddon->Sequences.Append(MoveTemp(ModuleSequences));
}

void FMASkillSequenceAssembler::Finalize(UMASkillModule& TargetModule)
{
	UMASkillModuleSequenceAddon* SequenceAddon =
		TargetModule.FindMutableAddon<UMASkillModuleSequenceAddon>();
	if (!SequenceAddon) return;

	TMap<FString, int32> SequenceCounts;
	for (const FMASkillSequence& Sequence : SequenceAddon->Sequences)
	{
		if (Sequence.UsesSequenceSections())
		{
			SequenceCounts.FindOrAdd(Sequence.GetSequenceKey())++;
		}
	}

	TMap<FString, int32> SequenceOffsets;
	for (FMASkillSequence& Sequence : SequenceAddon->Sequences)
	{
		if (!Sequence.UsesSequenceSections()) continue;

		const FString SequenceKey = Sequence.GetSequenceKey();
		const int32 InitialSequenceIndex = SequenceOffsets.FindRef(SequenceKey) + 1;
		SequenceOffsets.Add(SequenceKey, InitialSequenceIndex);
		Sequence.InitialSequenceIndex = InitialSequenceIndex;
		Sequence.SequenceAdvanceCount = SequenceCounts.FindRef(SequenceKey);
	}
}
