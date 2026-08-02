#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"

class UMASkillModuleAddon;
class UMASkillModule;
struct FMASkillModuleData;

enum class EMASkillAddonAssemblyStage : uint8
{
	ModuleComposition,
	SkillAssembly
};

/** Accumulates addon contributions without knowing any concrete addon types. */
struct FMASkillAddonAssembler
{
	static void AppendFrom(
		UObject& ResultOuter,
		FMASkillModuleData& ResultData,
		const UMASkillModule& SourceModule,
		EMASkillAddonAssemblyStage Stage,
		const FMASkillScopes& SourceScopes = FMASkillScopes());

	static void Finalize(
		FMASkillModuleData& ResultData,
		EMASkillAddonAssemblyStage Stage);
};
