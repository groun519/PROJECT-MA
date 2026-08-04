#pragma once

#include "CoreMinimal.h"
#include "MASkillComposedModule.generated.h"

class UMASkillModule;
class UMASkillModuleInstance;
struct FMASkillModuleGroup;

/** Owns the transient result and runtime bindings of one composed module. */
USTRUCT()
struct FMASkillComposedModule
{
	GENERATED_BODY()

	const UMASkillModule* Resolve(
		UMASkillModuleInstance& ModuleInstance,
		const FMASkillModuleGroup& ModuleGroup);
	void Reset(UMASkillModuleInstance& ModuleInstance);

private:
	UPROPERTY(Transient)
	TObjectPtr<UMASkillModule> Result;
};
