#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void MASkillModuleAddonStatics::ForEachAddon(
	const UMASkillModuleInstance& ModuleInstance,
	TFunctionRef<void(const UMASkillModuleAddon&)> Func)
{
	if (const UMASkillDefinition* Definition = ModuleInstance.GetDefinition())
	{
		Definition->ForEachAddon(Func);
	}
}
