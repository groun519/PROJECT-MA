#include "GAS/Skill/Definition/MASkillModuleAssembler.h"

#include "GAS/Skill/Definition/Assembly/MASkillAddonAssembler.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Module/MASkillModule.h"

UMASkillModule* FMASkillModuleAssembler::Assemble(
	UObject* Outer,
	const FMASkillModuleGroup& ModuleGroup)
{
	check(Outer);
	check(ModuleGroup.RootModule);

	UMASkillModule* ComposedModule = NewObject<UMASkillModule>(Outer);
	check(ComposedModule);
	FMASkillModuleData& ComposedData = ComposedModule->BeginAssembly();

	// Identity and display stay rooted in the parent; submodules only contribute features.
	ComposedData = ModuleGroup.RootModule->GetModuleData();
	ComposedData.ModuleVisualTags.Reset();
	ComposedData.Addons.Reset();
	ComposedData.Payloads.Reset();
	ComposedData.AssembledSubIcon = nullptr;

	const auto AppendSourceModule = [&](const UMASkillModule& SourceModule)
	{
		ComposedData.ModuleVisualTags.AppendTags(
			SourceModule.GetModuleData().ModuleVisualTags);
		ComposedData.Payloads.Append(SourceModule.GetModuleData().Payloads);
		FMASkillAddonAssembler::AppendFrom(
			*ComposedModule,
			ComposedData,
			SourceModule,
			EMASkillAddonAssemblyStage::ModuleComposition);
	};

	AppendSourceModule(*ModuleGroup.RootModule);
	for (const UMASkillModule* SubModule : ModuleGroup.SubModules)
	{
		if (SubModule) AppendSourceModule(*SubModule);
	}
	FMASkillAddonAssembler::Finalize(
		ComposedData,
		EMASkillAddonAssemblyStage::ModuleComposition);

	return ComposedModule;
}
