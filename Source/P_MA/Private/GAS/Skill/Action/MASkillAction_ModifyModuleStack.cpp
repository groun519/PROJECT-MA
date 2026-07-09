#include "GAS/Skill/Action/MASkillAction_ModifyModuleStack.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillAction_ModifyModuleStack::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent&,
	const FMASkillScopes& Scopes)
{
	UMASkillModuleInstance* ModuleInstance = Scopes.Module.Get();
	if (!ModuleInstance || !ModuleInstance->IsStackEnabled()) return;

	const int32 PreviousStack = ModuleInstance->GetStack();
	switch (Operation)
	{
	case EMASkillModuleStackOperation::Add:
		ModuleInstance->AddStack(Value);
		break;

	case EMASkillModuleStackOperation::Set:
		ModuleInstance->SetStack(Value);
		break;

	case EMASkillModuleStackOperation::Clear:
		ModuleInstance->ClearStack();
		break;
	}

	if (ModuleInstance->GetStack() == PreviousStack) return;

	const FGameplayTag StackChangedEventTag = UMAAbilitySystemStatics::GetModuleStackChangedEventTag();
	const UMASkillDefinition* ModuleDefinition = ModuleInstance->GetDefinition();
	if (!ModuleDefinition || !ModuleDefinition->HasEventSource(StackChangedEventTag)) return;

	UMASkillEventRoutingStatics::TryNotifySkillEvent(
		&OwnerAbility,
		StackChangedEventTag,
		FMASkillScopes(ModuleInstance, Scopes.Skill.Get()));
}

