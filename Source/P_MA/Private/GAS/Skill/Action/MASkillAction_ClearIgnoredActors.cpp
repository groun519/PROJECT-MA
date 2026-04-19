#include "GAS/Skill/Action/MASkillAction_ClearIgnoredActors.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GAS/Skill/Runtime/MASkillRuntimeContext.h"

void UMASkillAction_ClearIgnoredActors::Execute(UMASkillAbility& OwnerAbility, FSkillRuntimeContext& RuntimeContext, FMASkillPayloadStore& PayloadStore, const FGameplayEventData& Payload)
{
	(void)OwnerAbility;
	(void)PayloadStore;
	(void)Payload;
	RuntimeContext.ClearIgnoredActors();
}
