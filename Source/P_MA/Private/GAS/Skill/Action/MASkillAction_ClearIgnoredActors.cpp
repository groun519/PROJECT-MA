#include "GAS/Skill/Action/MASkillAction_ClearIgnoredActors.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_ClearIgnoredActors::Execute(FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	(void)Payload;
	RuntimeContext.ClearIgnoredActors();
}
