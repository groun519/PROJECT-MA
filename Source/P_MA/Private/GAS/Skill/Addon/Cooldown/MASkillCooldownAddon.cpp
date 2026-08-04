#include "GAS/Skill/Addon/Cooldown/MASkillCooldownAddon.h"

UMASkillModuleAddon* UMASkillCooldownAddon::AssembleInto(
	UObject& ResultOuter,
	UMASkillModuleAddon* ResultAddon,
	const EMASkillAddonAssemblyStage,
	const FMASkillScopes&) const
{
	if (CooldownSeconds <= 0.f) return ResultAddon;

	UMASkillCooldownAddon* Result = ResultAddon
		? static_cast<UMASkillCooldownAddon*>(ResultAddon)
		: NewObject<UMASkillCooldownAddon>(&ResultOuter, GetClass());
	Result->CooldownSeconds += CooldownSeconds;
	return Result;
}
