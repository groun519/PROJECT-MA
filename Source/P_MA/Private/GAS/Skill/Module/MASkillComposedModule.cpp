#include "GAS/Skill/Module/MASkillComposedModule.h"

#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GAS/Skill/Definition/MASkillModuleAssembler.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

const UMASkillModule* FMASkillComposedModule::Resolve(
	UMASkillModuleInstance& ModuleInstance,
	const FMASkillModuleGroup& ModuleGroup)
{
	if (!ModuleGroup.RootModule) return nullptr;
	if (Result) return Result;

	Result = FMASkillModuleAssembler::Assemble(&ModuleInstance, ModuleGroup);
	Result->ForEachAddon([&ModuleInstance](const UMASkillModuleAddon& Addon)
	{
		Addon.BindModule(ModuleInstance);
	});
	return Result;
}

void FMASkillComposedModule::Reset(UMASkillModuleInstance& ModuleInstance)
{
	if (!Result) return;

	Result->ForEachAddon([&ModuleInstance](const UMASkillModuleAddon& Addon)
	{
		Addon.UnbindModule(ModuleInstance);
	});
	Result = nullptr;
}
