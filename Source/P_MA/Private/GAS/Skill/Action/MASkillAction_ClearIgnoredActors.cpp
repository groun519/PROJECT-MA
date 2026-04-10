#include "GAS/Skill/Action/MASkillAction_ClearIgnoredActors.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_ClearIgnoredActors::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, const FGameplayEventData& Payload)
{
	(void)OwnerAbility;
	(void)Payload;
	RuntimeContext.ClearIgnoredActors();
}
