#include "GAS/Skill/Event/Source/MASkillEventSource_DamageDealt.h"

#include "GAS/MAAbilitySystemStatics.h"

UMASkillEventSource_DamageDealt::UMASkillEventSource_DamageDealt()
{
	EmittedTag = UMAAbilitySystemStatics::GetDamageDealtEventTag();
}
