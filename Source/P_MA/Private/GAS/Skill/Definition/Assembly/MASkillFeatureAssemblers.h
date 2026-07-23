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
	static void AppendDefinitions(
		UMASkillModule& TargetModule,
		const UMASkillModule& SourceModule);

	static void AppendToSkill(
		UMASkillModule& TargetModule,
		const UMASkillModule& SourceModule,
		const FMASkillScopes& TargetScopes);

private:
	static void AppendWithScopes(
		UMASkillModule& TargetModule,
		const UMASkillModule& SourceModule,
		const FMASkillScopes& TargetScopes);
};

struct FMASkillSequenceAssembler
{
	static void ComposeModule(
		UMASkillModule& TargetModule,
		const TArray<const UMASkillModule*>& SourceModules);

	static void AppendToSkill(
		UMASkillModule& TargetModule,
		const UMASkillModule& SourceModule,
		const FMASkillScopes& TargetScopes);

	static void Finalize(UMASkillModule& TargetModule);
};
