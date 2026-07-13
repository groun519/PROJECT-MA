#include "GAS/Skill/Addon/Event/MASkillModuleEventSourceAddon.h"

#include "GAS/Skill/Event/Source/MASkillEventSource.h"

bool UMASkillModuleEventSourceAddon::HasEventSource(const FGameplayTag& EventTag) const
{
	if (!EventTag.IsValid()) return false;

	for (const UMASkillEventSource* EventSource : EventSources)
	{
		if (EventSource && EventSource->GetEmittedTag() == EventTag) return true;
	}

	return false;
}
