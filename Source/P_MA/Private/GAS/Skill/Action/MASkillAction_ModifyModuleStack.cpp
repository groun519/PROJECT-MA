#include "GAS/Skill/Action/MASkillAction_ModifyModuleStack.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventSourceAddon.h"
#include "GAS/Skill/Addon/Stack/MASkillModuleStackAddon.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"

void UMASkillAction_ModifyModuleStack::Execute(
	AActor& Owner,
	UMASkillAbility* Ability,
	const FMASkillEvent& Event,
	const FMASkillScopes* Scopes)
{
	check(Ability && Scopes);
	UMASkillModuleInstance* ModuleInstance = Scopes->Module.Get();
	const UMASkillModule* Module = ModuleInstance ? ModuleInstance->GetRootModule() : nullptr;
	if (!ModuleInstance || !Module) return;

	const UMASkillModuleStackAddon* StackAddon = Module->GetStackAddon();
	if (!StackAddon) return;

	int32 ResolvedValue = Value;
	if (Operation != EMASkillModuleStackOperation::Clear && ValuePayloadTag.IsValid())
	{
		float PayloadValue = 0.f;
		if (!Event.GetPayloadAccess(*Scopes).Reader.TryGetScalar(ValuePayloadTag, PayloadValue)) return;
		ResolvedValue = FMath::RoundToInt(PayloadValue);
	}

	const bool bChanged = ModuleInstance->ModifyAddonRuntimeData<FMASkillModuleStackRuntimeData>(
		[this, StackAddon, ResolvedValue](FMASkillModuleStackRuntimeData& StackData)
	{
		int64 NewStackValue = StackData.Stack;
		switch (Operation)
		{
		case EMASkillModuleStackOperation::Add:
			NewStackValue += ResolvedValue;
			break;

		case EMASkillModuleStackOperation::Set:
			NewStackValue = ResolvedValue;
			break;

		case EMASkillModuleStackOperation::Clear:
			NewStackValue = 0;
			break;
		}

		const int32 NewStack = StackAddon->ClampStack(NewStackValue);
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
		Ability,
		StackChangedEventTag,
		FMASkillScopes(ModuleInstance, Scopes->Skill.Get()));
}

