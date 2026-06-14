#include "GAS/Skill/Definition/MASkillDefinition.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Event/Source/MASkillEventSource.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

FMASkillDefinitionIconData UMASkillDefinition::ResolveIconData(const UMAModuleQualityData* ModuleQualityData) const
{
	FMASkillDefinitionIconData IconData = DisplayData.IconData;
	if (!ModuleQualityData || ModuleQuality.Type == EMAModuleType::Elemental) return IconData;

	if (const FMAModuleTypeData* TypeData = ModuleQualityData->FindTypeData(ModuleQuality.Type))
	{
		IconData.IconColor = TypeData->IconColor;
		IconData.InnerColor = TypeData->InnerColor;
	}
	return IconData;
}

FLinearColor UMASkillDefinition::ResolveFrameColor(const UMAModuleQualityData* ModuleQualityData) const
{
	const FMAModuleRarityData* RarityData = ModuleQualityData
		? ModuleQualityData->FindRarityData(ModuleQuality.Rarity)
		: nullptr;
	return RarityData ? RarityData->Color : FLinearColor::White;
}

void UMASkillDefinition::PostLoad()
{
	Super::PostLoad();

	if (ExclusiveAssemblyTag_DEPRECATED.IsValid())
	{
		ExclusiveAssemblyTags.AddTag(ExclusiveAssemblyTag_DEPRECATED);
	}
	if (UniqueModuleEffectTag_DEPRECATED.IsValid())
	{
		ExclusiveAssemblyTags.AddTag(UniqueModuleEffectTag_DEPRECATED);
	}

	for (FMASkillEventBinding& EventBinding : EventBindings)
	{
		if (EventBinding.bUseLocalBinding)
		{
			EventBinding.BindingScope = EMASkillEventBindingScope::Module;
		}
		EventBinding.bUseLocalBinding = false;
	}
}

void UMASkillDefinition::ResetAssemblyData()
{
	DisplayData = FMASkillDefinitionDisplayData();
	AssembledSubIcon = nullptr;
	ExclusiveAssemblyTags.Reset();
	ExclusiveAssemblyTag_DEPRECATED = FGameplayTag();
	UniqueModuleEffectTag_DEPRECATED = FGameplayTag();
	ElementalTag = FGameplayTag();
	CooldownSeconds = 0.f;
	SkillSteps.Reset();
	EventSources.Reset();
	EventBindings.Reset();
	Payloads.Reset();
}

void UMASkillDefinition::AppendFrom(UMASkillModuleInstance* SourceModuleInstance)
{
	const UMASkillDefinition* SourceDefinition = SourceModuleInstance ? SourceModuleInstance->GetDefinition() : nullptr;
	if (!SourceDefinition) return;

	if (!ElementalTag.IsValid()
		&& SourceDefinition->ElementalTag.IsValid()
		&& SourceDefinition->ElementalTag != UMAAbilitySystemStatics::GetDefaultElementalTag())
	{
		ElementalTag = SourceDefinition->ElementalTag;
	}

	CooldownSeconds += SourceDefinition->CooldownSeconds;

	for (UMASkillStep* SkillStep : SourceDefinition->SkillSteps)
	{
		if (!SkillStep) continue;
		UMASkillStep* NewSkillStep = DuplicateObject<UMASkillStep>(SkillStep, this);
		if (!NewSkillStep) continue;

		NewSkillStep->SetBindingScope(SourceModuleInstance);
		SkillSteps.Add(NewSkillStep);
	}

	for (UMASkillEventSource* EventSource : SourceDefinition->EventSources)
	{
		if (!EventSource) continue;
		UMASkillEventSource* NewEventSource = DuplicateObject<UMASkillEventSource>(EventSource, this);
		if (!NewEventSource) continue;

		EventSources.Add(NewEventSource);
	}

	for (const FMASkillEventBinding& EventBinding : SourceDefinition->EventBindings)
	{
		FMASkillEventBinding NewEventBinding = EventBinding;
		NewEventBinding.BindingScopes.Module = SourceModuleInstance;
		NewEventBinding.BindingScopes.Skill = Cast<UMASkillModuleInstance>(GetOuter());
		NewEventBinding.Action = EventBinding.Action
			? DuplicateObject<UMASkillAction>(EventBinding.Action, this)
			: nullptr;
		EventBindings.Add(MoveTemp(NewEventBinding));
	}

	Payloads.Append(SourceDefinition->Payloads);
}

void UMASkillDefinition::FinalizeStepAssembly()
{
	TMap<FString, int32> SequenceOffsets;
	TMap<FString, int32> SequenceCounts;

	for (const UMASkillStep* SkillStep : SkillSteps)
	{
		if (!SkillStep || !SkillStep->UsesSequenceSections()) continue;

		SequenceCounts.FindOrAdd(SkillStep->GetSequenceSectionKey())++;
	}

	for (int32 StepIndex = 0; StepIndex < SkillSteps.Num(); ++StepIndex)
	{
		UMASkillStep* SkillStep = SkillSteps[StepIndex];
		if (!SkillStep) continue;

		const int32 NextStepIndex = SkillSteps.IsValidIndex(StepIndex + 1) ? StepIndex + 1 : INDEX_NONE;
		int32 NextMontageStepIndex = INDEX_NONE;
		for (int32 NextStepCandidateIndex = StepIndex + 1; NextStepCandidateIndex < SkillSteps.Num(); ++NextStepCandidateIndex)
		{
			const UMASkillStep* NextSkillStep = SkillSteps[NextStepCandidateIndex];
			if (!NextSkillStep || !NextSkillStep->ResolveStepMontage()) continue;

			NextMontageStepIndex = NextStepCandidateIndex;
			break;
		}

		int32 InitialSequenceIndex = 0;
		int32 SequenceAdvanceCount = 0;
		if (SkillStep->UsesSequenceSections())
		{
			const FString SequenceSectionKey = SkillStep->GetSequenceSectionKey();
			InitialSequenceIndex = SequenceOffsets.FindRef(SequenceSectionKey) + 1;
			SequenceOffsets.Add(SequenceSectionKey, InitialSequenceIndex);
			SequenceAdvanceCount = SequenceCounts.FindRef(SequenceSectionKey);
		}

		SkillStep->ConfigureAssembledStep(
			StepIndex,
			NextStepIndex,
			NextMontageStepIndex,
			InitialSequenceIndex,
			SequenceAdvanceCount);
	}
}
