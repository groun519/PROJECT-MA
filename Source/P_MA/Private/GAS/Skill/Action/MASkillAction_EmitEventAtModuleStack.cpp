#include "GAS/Skill/Action/MASkillAction_EmitEventAtModuleStack.h"

#include "GAS/Skill/Addon/Stack/MASkillModuleStackAddon.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillAction_EmitEventAtModuleStack::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	if (!EventTag.IsValid()) return;

	UMASkillModuleInstance* ModuleInstance = Scopes->Module.Get();
	const FMASkillModuleStackRuntimeData* StackData = ModuleInstance
		? ModuleInstance->GetAddonRuntimeData().Find<FMASkillModuleStackRuntimeData>()
		: nullptr;
	if (!StackData || StackData->Stack != RequiredStack) return;

	UMASkillEventRoutingStatics::TryNotifySkillEvent(
		Ability,
		EventTag,
		FMASkillScopes(ModuleInstance, Scopes->Skill.Get()));
}
