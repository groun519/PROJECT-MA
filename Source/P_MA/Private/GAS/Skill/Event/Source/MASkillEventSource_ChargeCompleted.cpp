#include "GAS/Skill/Event/Source/MASkillEventSource_ChargeCompleted.h"

#include "GAS/MAAbilitySystemStatics.h"

UMASkillEventSource_ChargeCompleted::UMASkillEventSource_ChargeCompleted()
{
	EmittedTag = UMAAbilitySystemStatics::GetChargeCompletedEventTag();
}
