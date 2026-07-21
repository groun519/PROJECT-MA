#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"

class UMASkillModule;
class UMASkillModuleInstance;

struct FMASkillCooldownAssembler
{
	static void AppendFrom(
		UMASkillModule& TargetModule,
		const UMASkillModule& SourceModule);
};

struct FMASkillPayloadAssembler
{
	static void AppendFrom(
		UMASkillModule& TargetModule,
		const UMASkillModule& SourceModule);
};

struct FMASkillEventSourceAssembler
{
	static void AppendFrom(
		UMASkillModule& TargetModule,
		const UMASkillModule& SourceModule);
};

struct FMASkillEventBindingAssembler
{
	static void AppendFrom(
		UMASkillModule& TargetModule,
		const UMASkillModule& SourceModule,
		UMASkillModuleInstance& SourceModuleInstance,
		UMASkillModuleInstance& AssembledModuleInstance);
};

struct FMASkillSequenceAssembler
{
	static void AppendFrom(
		UMASkillModule& TargetModule,
		const UMASkillModule& SourceModule,
		const FMASkillScopes& TargetScopes);

	static void Finalize(UMASkillModule& TargetModule);
};
