#include "GAS/Skill/Event/Binding/MASkillEventBinding.h"

#include "GAS/Skill/Module/MASkillModuleInstance.h"

bool FMASkillEventBinding::CanExecute() const
{
	const UMASkillModuleInstance* ModuleInstance = BindingScopes.Module.Get();
	return !ModuleInstance
		|| (ModuleInstance->IsActive() && !ModuleInstance->IsCooldownActive());
}

bool FMASkillEventBinding::TryResolveScopes(
	const FMASkillScopes& SourceScopes,
	FMASkillScopes& OutScopes) const
{
	OutScopes = FMASkillScopes();

	switch (BindingScope)
	{
	case EMASkillEventBindingScope::Module:
		if (!SourceScopes.Module || SourceScopes.Module != BindingScopes.Module) return false;
		OutScopes = BindingScopes;
		return true;

	case EMASkillEventBindingScope::Skill:
		if (SourceScopes.Skill != BindingScopes.Skill) return false;
		OutScopes = BindingScopes;
		return true;

	case EMASkillEventBindingScope::Global:
		OutScopes = BindingScopes;
		return true;
	}

	return false;
}
