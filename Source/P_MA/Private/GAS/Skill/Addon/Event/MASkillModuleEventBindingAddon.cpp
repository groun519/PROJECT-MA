#include "GAS/Skill/Addon/Event/MASkillModuleEventBindingAddon.h"

UMASkillModuleAddon* UMASkillModuleEventBindingAddon::AssembleInto(
	UObject& ResultOuter,
	UMASkillModuleAddon* ResultAddon,
	const EMASkillAddonAssemblyStage Stage,
	const FMASkillScopes& SourceScopes) const
{
	if (EventBindings.IsEmpty()) return ResultAddon;

	UMASkillModuleEventBindingAddon* Result = ResultAddon
		? static_cast<UMASkillModuleEventBindingAddon*>(ResultAddon)
		: NewObject<UMASkillModuleEventBindingAddon>(&ResultOuter, GetClass());
	const FMASkillScopes BindingScopes = Stage == EMASkillAddonAssemblyStage::SkillAssembly
		? SourceScopes
		: FMASkillScopes();
	for (const FMASkillEventBinding& EventBinding : EventBindings)
	{
		FMASkillEventBinding& ResultBinding = Result->EventBindings.Add_GetRef(EventBinding);
		ResultBinding.BindingScopes = BindingScopes;
	}
	return Result;
}
