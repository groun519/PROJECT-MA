#include "GAS/Skill/Addon/Event/MASkillModuleEventSourceAddon.h"

#include "GAS/Skill/Event/Source/MASkillEventSource.h"

UMASkillModuleAddon* UMASkillModuleEventSourceAddon::AssembleInto(
	UObject& ResultOuter,
	UMASkillModuleAddon* ResultAddon,
	const EMASkillAddonAssemblyStage,
	const FMASkillScopes&) const
{
	UMASkillModuleEventSourceAddon* Result = ResultAddon
		? static_cast<UMASkillModuleEventSourceAddon*>(ResultAddon)
		: nullptr;
	for (UMASkillEventSource* EventSource : EventSources)
	{
		if (!EventSource) continue;
		if (!Result)
		{
			Result = NewObject<UMASkillModuleEventSourceAddon>(&ResultOuter, GetClass());
		}

		UMASkillEventSource* NewEventSource =
			DuplicateObject<UMASkillEventSource>(EventSource, Result);
		if (NewEventSource) Result->EventSources.Add(NewEventSource);
	}
	return Result;
}

bool UMASkillModuleEventSourceAddon::HasEventSource(const FGameplayTag& EventTag) const
{
	if (!EventTag.IsValid()) return false;

	for (const UMASkillEventSource* EventSource : EventSources)
	{
		if (EventSource && EventSource->GetEmittedTag() == EventTag) return true;
	}

	return false;
}
