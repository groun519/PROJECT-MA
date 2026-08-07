#include "GAS/Skill/Addon/Cooldown/MASkillCooldownAddon.h"

UMASkillModuleAddon* UMASkillCooldownAddon::AssembleInto(
	UObject& ResultOuter,
	UMASkillModuleAddon* ResultAddon,
	const EMASkillAddonAssemblyStage,
	const FMASkillScopes&) const
{
	if (CooldownSeconds == 0.f
		&& CooldownMultiplier == 1.f
		&& CooldownOffsetSeconds == 0.f)
	{
		return ResultAddon;
	}

	UMASkillCooldownAddon* Result = ResultAddon
		? static_cast<UMASkillCooldownAddon*>(ResultAddon)
		: NewObject<UMASkillCooldownAddon>(&ResultOuter, GetClass());
	Result->CooldownSeconds += CooldownSeconds;
	Result->CooldownMultiplier *= CooldownMultiplier;
	Result->CooldownOffsetSeconds += CooldownOffsetSeconds;
	return Result;
}

bool UMASkillCooldownAddon::Finalize(const EMASkillAddonAssemblyStage Stage)
{
	if (Stage != EMASkillAddonAssemblyStage::SkillAssembly) return true;

	CooldownSeconds = FMath::Max(GetCooldownSeconds(), 0.f);
	CooldownMultiplier = 1.f;
	CooldownOffsetSeconds = 0.f;
	return CooldownSeconds > 0.f;
}
