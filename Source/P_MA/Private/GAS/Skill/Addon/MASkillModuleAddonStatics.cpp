#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"

#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void MASkillModuleAddonStatics::ForEachAddon(
	const UMASkillModuleInstance& ModuleInstance,
	TFunctionRef<void(const UMASkillModuleAddon&)> Func)
{
	if (const UMASkillModule* Module = ModuleInstance.GetModule())
	{
		Module->ForEachAddon(Func);
	}
}
