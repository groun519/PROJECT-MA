#include "GAS/Skill/Event/MASkillEventSource.h"

#include "GAS/Skill/MASkillAbility.h"

void UMASkillEventSource::CreateRuntimeSources(UMASkillAbility* SkillAbility,
	const TArray<TObjectPtr<UMASkillEventSource>>& SourceTemplates,
	TArray<TObjectPtr<UMASkillEventSource>>& OutRuntimeSources)
{
	OutRuntimeSources.Reset();
	if (!SkillAbility) return;

	for (UMASkillEventSource* EventSourceTemplate : SourceTemplates)
	{
		if (!EventSourceTemplate) continue;

		UMASkillEventSource* RuntimeEventSource = DuplicateObject<UMASkillEventSource>(EventSourceTemplate, SkillAbility);
		if (!RuntimeEventSource) continue;

		RuntimeEventSource->StartSource(SkillAbility);
		OutRuntimeSources.Add(RuntimeEventSource);
	}
}

void UMASkillEventSource::StopRuntimeSources(TArray<TObjectPtr<UMASkillEventSource>>& RuntimeSources)
{
	for (UMASkillEventSource* RuntimeEventSource : RuntimeSources)
	{
		if (!RuntimeEventSource) continue;
		RuntimeEventSource->StopSource();
	}

	RuntimeSources.Reset();
}

void UMASkillEventSource::EmitEvent() const
{
	if (OwnerSkillAbility) OwnerSkillAbility->HandleSkillTagEvent(EmittedTag);
}
