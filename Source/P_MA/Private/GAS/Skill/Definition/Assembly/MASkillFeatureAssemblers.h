#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"

class UMASkillDefinition;
class UMASkillModuleInstance;

struct FMASkillCooldownAssembler
{
	static void AppendFrom(
		UMASkillDefinition& TargetDefinition,
		const UMASkillDefinition& SourceDefinition);
};

struct FMASkillPayloadAssembler
{
	static void AppendFrom(
		UMASkillDefinition& TargetDefinition,
		const UMASkillDefinition& SourceDefinition);
};

struct FMASkillEventSourceAssembler
{
	static void AppendFrom(
		UMASkillDefinition& TargetDefinition,
		const UMASkillDefinition& SourceDefinition);
};

struct FMASkillEventBindingAssembler
{
	static void AppendFrom(
		UMASkillDefinition& TargetDefinition,
		const UMASkillDefinition& SourceDefinition,
		UMASkillModuleInstance& SourceModuleInstance,
		UMASkillModuleInstance& AssembledModuleInstance);
};

struct FMASkillSequenceAssembler
{
	static void AppendFrom(
		UMASkillDefinition& TargetDefinition,
		const UMASkillDefinition& SourceDefinition,
		const FMASkillScopes& TargetScopes);

	static void Finalize(UMASkillDefinition& TargetDefinition);
};
