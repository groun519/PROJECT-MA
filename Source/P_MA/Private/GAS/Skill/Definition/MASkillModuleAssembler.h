#pragma once

#include "CoreMinimal.h"

class UMASkillModule;
struct FMASkillModuleGroup;

/** Composes one root module and its scope-free submodules into one skill contribution. */
struct FMASkillModuleAssembler
{
	static UMASkillModule* Assemble(
		UObject* Outer,
		const FMASkillModuleGroup& ModuleGroup);
};
