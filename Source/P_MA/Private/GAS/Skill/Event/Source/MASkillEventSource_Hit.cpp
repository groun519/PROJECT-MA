#include "GAS/Skill/Event/Source/MASkillEventSource_Hit.h"

#include "GAS/MAAbilitySystemStatics.h"

UMASkillEventSource_Hit::UMASkillEventSource_Hit()
{
	EmittedTag = UMAAbilitySystemStatics::GetHitEventTag();
}
