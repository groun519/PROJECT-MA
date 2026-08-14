#include "GAS/Skill/Action/MASkillAction_AccumulateModuleStackFromPayload.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Addon/MASkillModuleAddonStatics.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventSourceAddon.h"
#include "GAS/Skill/Addon/Stack/MASkillModuleStackAddon.h"
#include "GAS/Skill/Event/Routing/MASkillEventRoutingStatics.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"

void UMASkillAction_AccumulateModuleStackFromPayload::Execute(
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
	if (!StackAddon || !ValuePayloadTag.IsValid() || !AccumulatedPayloadTag.IsValid()) return;

	FMASkillPayloadAccess Payloads = Event.GetPayloadAccess(*Scopes);
	float Value = 0.f;
	if (!Payloads.Reader.TryGetScalar(ValuePayloadTag, Value) || !FMath::IsFinite(Value)) return;

	float AccumulatedValue = 0.f;
	Payloads.Reader.TryGetScalar(
		EMASkillPayloadScope::Module,
		AccumulatedPayloadTag,
		AccumulatedValue);
	AccumulatedValue += Value;
	Payloads.Writer.SetScalar(
		EMASkillPayloadScope::Module,
		AccumulatedPayloadTag,
		AccumulatedValue,
		/* bKeepValueOnPayloadReset */ true);

	const int32 ResolvedStack = StackAddon->ClampStack(
		FMath::RoundToInt(StackCalculation.Calculate(AccumulatedValue)));
	const bool bChanged = ModuleInstance->ModifyAddonRuntimeData<FMASkillModuleStackRuntimeData>(
		[ResolvedStack](FMASkillModuleStackRuntimeData& StackData)
	{
		if (StackData.Stack == ResolvedStack) return false;

		StackData.Stack = ResolvedStack;
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
