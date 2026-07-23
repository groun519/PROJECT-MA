#include "GAS/Skill/Definition/MASkillModuleAssembler.h"

#include "GAS/Skill/Definition/Assembly/MASkillFeatureAssemblers.h"
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
	ComposedModule->ResetAssemblyData();

	// Identity and display stay rooted in the parent; submodules only contribute features.
	ComposedModule->ModuleData = ModuleGroup.RootModule->GetModuleData();
	ComposedModule->ModuleData.ModuleVisualTags.Reset();
	ComposedModule->ModuleData.Addons.Reset();
	ComposedModule->ModuleData.Payloads.Reset();
	ComposedModule->ModuleData.AssembledSubIcon = nullptr;

	TArray<const UMASkillModule*> SourceModules;
	SourceModules.Reserve(1 + ModuleGroup.SubModules.Num());
	SourceModules.Add(ModuleGroup.RootModule);
	for (const UMASkillModule* SubModule : ModuleGroup.SubModules)
	{
		if (SubModule) SourceModules.Add(SubModule);
	}

	for (const UMASkillModule* SourceModule : SourceModules)
	{
		ComposedModule->ModuleData.ModuleVisualTags.AppendTags(
			SourceModule->GetModuleData().ModuleVisualTags);
		FMASkillCooldownAssembler::AppendFrom(*ComposedModule, *SourceModule);
		FMASkillPayloadAssembler::AppendFrom(*ComposedModule, *SourceModule);
		FMASkillEventSourceAssembler::AppendFrom(*ComposedModule, *SourceModule);
		FMASkillEventBindingAssembler::AppendDefinitions(*ComposedModule, *SourceModule);
	}
	FMASkillSequenceAssembler::ComposeModule(*ComposedModule, SourceModules);

	return ComposedModule;
}
