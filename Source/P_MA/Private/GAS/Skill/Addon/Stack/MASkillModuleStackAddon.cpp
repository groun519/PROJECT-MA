#include "GAS/Skill/Addon/Stack/MASkillModuleStackAddon.h"

#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Module/MASkillModuleAddonRuntimeData.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"

void UMASkillModuleStackAddon::InitializeRuntimeData(FMASkillModuleAddonRuntimeData& RuntimeData) const
{
	if (RuntimeData.Find<FMASkillModuleStackRuntimeData>()) return;

	FMASkillModuleStackRuntimeData& StackData =
		RuntimeData.FindOrAdd<FMASkillModuleStackRuntimeData>();
	StackData.Stack = ClampStack(InitialStack);
}

void UMASkillModuleStackAddon::ApplyPayloadMirror(
	const FMASkillModuleAddonRuntimeData& RuntimeData,
	FMASkillPayloadStore& PayloadStore) const
{
	const FMASkillModuleStackRuntimeData* StackData =
		RuntimeData.Find<FMASkillModuleStackRuntimeData>();
	if (!StackData) return;

	PayloadStore.SetScalar(
		UMAAbilitySystemStatics::GetModuleStackTag(),
		static_cast<float>(StackData->Stack));
}

bool UMASkillModuleStackAddon::TryResolveSocketText(
	const FMASkillModuleAddonRuntimeData& RuntimeData,
	FText& OutText) const
{
	if (!ShouldShowStackText()) return false;

	const FMASkillModuleStackRuntimeData* StackData =
		RuntimeData.Find<FMASkillModuleStackRuntimeData>();
	if (!StackData) return false;

	OutText = FText::AsNumber(StackData->Stack);
	return true;
}

int32 UMASkillModuleStackAddon::ClampStack(int32 Value) const
{
	return FMath::Clamp(Value, MinStack, MaxStack);
}
