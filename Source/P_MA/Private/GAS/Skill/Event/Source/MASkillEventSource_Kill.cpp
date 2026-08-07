#include "GAS/Skill/Event/Source/MASkillEventSource_Kill.h"

#include "GAS/MAAbilitySystemStatics.h"

UMASkillEventSource_Kill::UMASkillEventSource_Kill()
{
	EmittedTag = UMAAbilitySystemStatics::GetKillEventTag();
}
