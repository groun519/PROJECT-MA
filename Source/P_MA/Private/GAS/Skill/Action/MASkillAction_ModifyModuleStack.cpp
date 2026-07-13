#include "GAS/Skill/Action/MASkillAction_ModifyModuleStack.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventSourceAddon.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Addon/Stack/MASkillModuleStackAddon.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillAction_ModifyModuleStack::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent&,
	const FMASkillScopes& Scopes)
{
	UMASkillModuleInstance* ModuleInstance = Scopes.Module.Get();
	const UMASkillDefinition* ModuleDefinition = ModuleInstance ? ModuleInstance->GetDefinition() : nullptr;
	if (!ModuleInstance || !ModuleDefinition) return;

	const UMASkillModuleStackAddon* StackAddon = ModuleDefinition->GetStackAddon();
	if (!StackAddon) return;

	const bool bChanged = ModuleInstance->ModifyAddonRuntimeData<FMASkillModuleStackRuntimeData>(
		[this, StackAddon](FMASkillModuleStackRuntimeData& StackData)
	{
		int32 NewStack = StackData.Stack;
		switch (Operation)
		{
		case EMASkillModuleStackOperation::Add:
			NewStack = StackData.Stack + Value;
			break;

		case EMASkillModuleStackOperation::Set:
			NewStack = Value;
			break;

		case EMASkillModuleStackOperation::Clear:
			NewStack = 0;
			break;
		}

		NewStack = StackAddon->ClampStack(NewStack);
		if (StackData.Stack == NewStack) return false;

		StackData.Stack = NewStack;
		return true;
	});
	if (!bChanged) return;

	const FGameplayTag StackChangedEventTag = UMAAbilitySystemStatics::GetModuleStackChangedEventTag();
	const UMASkillModuleEventSourceAddon* EventSourceAddon =
		MASkillModuleAddonStatics::FindAddon<UMASkillModuleEventSourceAddon>(*ModuleInstance);
	if (!EventSourceAddon || !EventSourceAddon->HasEventSource(StackChangedEventTag)) return;

	UMASkillEventRoutingStatics::TryNotifySkillEvent(
		&OwnerAbility,
		StackChangedEventTag,
		FMASkillScopes(ModuleInstance, Scopes.Skill.Get()));
}

