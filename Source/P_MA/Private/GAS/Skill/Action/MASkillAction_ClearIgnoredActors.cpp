#include "GAS/Skill/Action/MASkillAction_ClearIgnoredActors.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_ClearIgnoredActors::Execute(UMASkillAbility* SkillAbility, FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	(void)SkillAbility;
	(void)Payload;
	RuntimeContext.ClearIgnoredActors();
}
