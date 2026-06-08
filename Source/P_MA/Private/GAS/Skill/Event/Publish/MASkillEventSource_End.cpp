#include "GAS/Skill/Event/Publish/MASkillEventSource_End.h"

void UMASkillEventSource_End::StartSource(UMASkillAbility* SkillAbility)
{
	Super::StartSource(SkillAbility);
	bStarted = true;
}

void UMASkillEventSource_End::StopSource()
{
	if (!bStarted) return;

	bStarted = false;
	EmitEvent();
}
