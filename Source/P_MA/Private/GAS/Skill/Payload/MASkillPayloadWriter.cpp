#include "GAS/Skill/Payload/MASkillPayloadWriter.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

void UMASkillPayloadWriter_Static::WritePayload(UMASkillAbility& SkillAbility, const FGameplayEventData& EventData, UMASkillModuleInstance* EventScope) const
{
	(void)EventData;

	FMASkillPayloadStore& PayloadStore = EventScope
		? EventScope->GetPayloadStore()
		: SkillAbility.GetAssembledModulePayloadStore();
	for (const FMASkillPayloadEntry& Payload : Payloads)
	{
		Payload.ApplyTo(PayloadStore);
	}
}
