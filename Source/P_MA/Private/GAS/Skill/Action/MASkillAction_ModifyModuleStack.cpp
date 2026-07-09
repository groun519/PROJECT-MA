#include "GAS/Skill/Action/MASkillAction_ModifyModuleStack.h"

#include "GAS/MAAbilitySystemStatics.h"
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
	if (!ModuleInstance || !ModuleDefinition || !ModuleDefinition->IsStackEnabled()) return;

	const UMASkillModuleStackAddon* StackAddon = ModuleDefinition->GetStackAddon();
	auto ClampStack = [StackAddon](int32 Stack)
	{
		return StackAddon ? StackAddon->ClampStack(Stack) : FMath::Clamp(Stack, 0, 999);
	};

	const bool bChanged = ModuleInstance->ModifyAddonRuntimeData<FMASkillModuleStackRuntimeData>(
		[this, &ClampStack](FMASkillModuleStackRuntimeData& StackData)
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

		NewStack = ClampStack(NewStack);
		if (StackData.Stack == NewStack) return false;

		StackData.Stack = NewStack;
		return true;
	});
	if (!bChanged) return;

	const FGameplayTag StackChangedEventTag = UMAAbilitySystemStatics::GetModuleStackChangedEventTag();
	if (!ModuleDefinition->HasEventSource(StackChangedEventTag)) return;

	UMASkillEventRoutingStatics::TryNotifySkillEvent(
		&OwnerAbility,
		StackChangedEventTag,
		FMASkillScopes(ModuleInstance, Scopes.Skill.Get()));
}

