#include "GAS/Skill/Event/Publish/MASkillEventSource_Hit.h"

UMASkillEventSource_Hit::UMASkillEventSource_Hit()
{
	EmittedTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.Hit"));
}
