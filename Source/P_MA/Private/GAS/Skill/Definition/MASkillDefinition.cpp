#include "GAS/Skill/Definition/MASkillDefinition.h"

#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Event/Runtime/MASkillRuntimeScope.h"
#include "GAS/Skill/Event/Publish/MASkillEventSource.h"

namespace
{
FGameplayTag GetDefaultElementalTag()
{
	return FGameplayTag::RequestGameplayTag(TEXT("Elemental.Default"));
}
}

void UMASkillDefinition::ResetAssemblyData()
{
	ElementalTag = FGameplayTag();
	SkillSteps.Reset();
	EventSources.Reset();
	EventBindings.Reset();
	Payloads.Reset();
	bIsRuntimeAssembledDefinition = true;
}

void UMASkillDefinition::AppendFrom(const UMASkillDefinition& Other)
{
	UMASkillRuntimeScope* RuntimeScope = NewObject<UMASkillRuntimeScope>(this);

	if (Other.ElementalTag.IsValid() && Other.ElementalTag != GetDefaultElementalTag())
	{
		ElementalTag = Other.ElementalTag;
	}

	for (UMASkillStep* SkillStep : Other.SkillSteps)
	{
		if (!SkillStep) continue;
		UMASkillStep* NewSkillStep = DuplicateObject<UMASkillStep>(SkillStep, this);
		if (!NewSkillStep) continue;

		NewSkillStep->SetRuntimeScope(RuntimeScope);
		SkillSteps.Add(NewSkillStep);
	}

	for (UMASkillEventSource* EventSource : Other.EventSources)
	{
		if (!EventSource) continue;
		UMASkillEventSource* NewEventSource = DuplicateObject<UMASkillEventSource>(EventSource, this);
		if (!NewEventSource) continue;

		NewEventSource->SetRuntimeScope(RuntimeScope);
		EventSources.Add(NewEventSource);
	}

	for (const FMASkillGameplayEventBinding& EventBinding : Other.EventBindings)
	{
		FMASkillGameplayEventBinding NewEventBinding = EventBinding;
		NewEventBinding.RuntimeScope = EventBinding.bUseLocalBinding
			? RuntimeScope
			: nullptr;
		NewEventBinding.Action = EventBinding.Action
			? DuplicateObject<UMASkillAction>(EventBinding.Action, this)
			: nullptr;
		EventBindings.Add(MoveTemp(NewEventBinding));
	}

	Payloads.Append(Other.Payloads);
}


