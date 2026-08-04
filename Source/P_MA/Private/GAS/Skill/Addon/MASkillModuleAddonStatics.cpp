#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"

#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void MASkillModuleAddonStatics::ForEachAddon(
	const UMASkillModuleInstance& ModuleInstance,
	TFunctionRef<void(const UMASkillModuleAddon&)> Func)
{
	const auto VisitModule = [&Func](const UMASkillModule* Module)
	{
		if (Module) Module->ForEachAddon(Func);
	};

	const FMASkillModuleGroup& ModuleGroup = ModuleInstance.GetModuleGroup();
	VisitModule(ModuleGroup.RootModule);
	for (const UMASkillModule* SubModule : ModuleGroup.SubModules)
	{
		VisitModule(SubModule);
	}
}
