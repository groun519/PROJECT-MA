#include "GAS/Skill/Event/Publish/MASkillGameplayEventScope.h"

#include "GAS/Skill/Event/Runtime/MASkillRuntimeScope.h"

void MASkillGameplayEventScope::InjectRuntimeScope(FGameplayEventData& Payload, UMASkillRuntimeScope* InRuntimeScope)
{
	Payload.OptionalObject = InRuntimeScope;
}

const UMASkillRuntimeScope* MASkillGameplayEventScope::ExtractRuntimeScope(const FGameplayEventData& Payload)
{
	return Cast<UMASkillRuntimeScope>(Payload.OptionalObject);
}

