#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Payload/MASkillPayloadReader.h"
#include "GAS/Skill/Payload/MASkillPayloadWriter.h"

struct P_MA_API FMASkillPayloadAccess
{
	FMASkillPayloadAccess() = default;

	FMASkillPayloadAccess(
		const FMASkillPayloadStore* EventPayloads,
		const FMASkillPayloadStore* SkillPayloads,
		const FMASkillPayloadStore* ModulePayloads)
		: Reader(EventPayloads, SkillPayloads, ModulePayloads)
	{
	}

	FMASkillPayloadAccess(
		const FMASkillPayloadStore* EventPayloads,
		FMASkillPayloadStore* SkillPayloads,
		FMASkillPayloadStore* ModulePayloads)
		: Reader(EventPayloads, SkillPayloads, ModulePayloads)
		, Writer(SkillPayloads, ModulePayloads)
	{
	}

	FMASkillPayloadReader Reader;
	FMASkillPayloadWriter Writer;
};
