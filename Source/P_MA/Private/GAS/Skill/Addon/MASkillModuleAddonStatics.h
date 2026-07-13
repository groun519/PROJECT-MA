#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Addon/MASkillModuleAddon.h"

class UMASkillModuleInstance;

class P_MA_API MASkillModuleAddonStatics final
{
public:
	static void ForEachAddon(
		const UMASkillModuleInstance& ModuleInstance,
		TFunctionRef<void(const UMASkillModuleAddon&)> Func);

	template<typename AddonType>
	static const AddonType* FindAddon(const UMASkillModuleInstance& ModuleInstance)
	{
		static_assert(TIsDerivedFrom<AddonType, UMASkillModuleAddon>::IsDerived,
			"AddonType must derive from UMASkillModuleAddon.");

		const AddonType* FoundAddon = nullptr;
		ForEachAddon(ModuleInstance, [&FoundAddon](const UMASkillModuleAddon& Addon)
		{
			if (!FoundAddon) FoundAddon = Cast<AddonType>(&Addon);
		});
		return FoundAddon;
	}

private:
	MASkillModuleAddonStatics() = delete;
};
