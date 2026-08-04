#include "GAS/Skill/Event/Source/MASkillEventSource.h"

void UMASkillEventSource::InitializeRuntime(UMASkillManagerComponent* SkillManager)
{
	if (!SkillManager || OwnerSkillManager == SkillManager) return;
	if (OwnerSkillManager) DeinitializeRuntime();

	OwnerSkillManager = SkillManager;
	StartSource();
}

void UMASkillEventSource::DeinitializeRuntime()
{
	if (!OwnerSkillManager) return;

	StopSource();
	OwnerSkillManager = nullptr;
}

bool UMASkillEventSource::HasSameRuntimeConfiguration(const UMASkillEventSource& Other) const
{
	return GetClass() == Other.GetClass() && EmittedTag == Other.EmittedTag;
}
