#include "GAS/Skill/Definition/MASkillDefinition.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Event/Publish/MASkillEventSource.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillDefinition::ResetAssemblyData()
{
	DisplayData = FMASkillDefinitionDisplayData();
	AssembledSubIcon = nullptr;
	ExclusiveAssemblyTag = FGameplayTag();
	ElementalTag = FGameplayTag();
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

		NewEventSource->SetBindingScope(SourceModuleInstance);
		EventSources.Add(NewEventSource);
	}

	for (const FMASkillGameplayEventBinding& EventBinding : SourceDefinition->EventBindings)
	{
		FMASkillGameplayEventBinding NewEventBinding = EventBinding;
		NewEventBinding.BindingScope = EventBinding.bUseLocalBinding
			? SourceModuleInstance
			: nullptr;
		NewEventBinding.Action = EventBinding.Action
			? DuplicateObject<UMASkillAction>(EventBinding.Action, this)
			: nullptr;
		EventBindings.Add(MoveTemp(NewEventBinding));
	}

	Payloads.Append(SourceDefinition->Payloads);
}

void UMASkillDefinition::RestoreBindingScopesFrom(const UMASkillDefinition& SourceDefinition)
{
	const int32 StepCount = FMath::Min(SkillSteps.Num(), SourceDefinition.SkillSteps.Num());
	for (int32 Index = 0; Index < StepCount; ++Index)
	{
		if (SkillSteps[Index] && SourceDefinition.SkillSteps[Index])
		{
			SkillSteps[Index]->SetBindingScope(SourceDefinition.SkillSteps[Index]->GetBindingScope());
		}
	}

	const int32 EventSourceCount = FMath::Min(EventSources.Num(), SourceDefinition.EventSources.Num());
	for (int32 Index = 0; Index < EventSourceCount; ++Index)
	{
		if (EventSources[Index] && SourceDefinition.EventSources[Index])
		{
			EventSources[Index]->SetBindingScope(SourceDefinition.EventSources[Index]->GetBindingScope());
		}
	}

	const int32 EventBindingCount = FMath::Min(EventBindings.Num(), SourceDefinition.EventBindings.Num());
	for (int32 Index = 0; Index < EventBindingCount; ++Index)
	{
		EventBindings[Index].BindingScope = SourceDefinition.EventBindings[Index].BindingScope;
	}
}
