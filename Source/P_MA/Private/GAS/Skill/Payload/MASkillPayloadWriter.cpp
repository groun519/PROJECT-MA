#include "GAS/Skill/Payload/MASkillPayloadWriter.h"

#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

void UMASkillPayloadWriter_Static::WritePayload(UMASkillAbility& SkillAbility, const FGameplayEventData& EventData, UMASkillModuleInstance*) const
{
	(void)EventData;

	FMASkillPayloadStore& PayloadStore = SkillAbility.GetAssembledModulePayloadStore();
	for (const FMASkillPayloadEntry& Payload : Payloads)
	{
		Payload.ApplyTo(PayloadStore);
	}
}
