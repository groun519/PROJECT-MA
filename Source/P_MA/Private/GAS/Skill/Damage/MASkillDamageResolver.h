#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"

class UMASkillAbility;
struct FMASkillPayloadStore;

class P_MA_API MASkillDamageResolver final
{
public:
	static FResolvedSkillHitEffects Resolve(UMASkillAbility& OwnerAbility, const FMASkillDamageConfig& DamageConfig);
	static FResolvedSkillHitEffects Resolve(
		UMASkillAbility& OwnerAbility,
		const FMASkillDamageConfig& DamageConfig,
		const FMASkillPayloadStore& PayloadStore);

private:
	MASkillDamageResolver() = delete;

	static void ApplyDamageOverTimeConfig(FGameplayEffectSpecHandle& SpecHandle, const FMASkillDamageOverTimeConfig& DamageOverTime);
	static FMADamageExecutionConfig ScaleDamageConfigForTick(const FMADamageExecutionConfig& DamageConfig, int32 TickCount);
	static void AppendElementalHitGameplayCueTag(UMASkillAbility& OwnerAbility, FGameplayTagContainer& TargetGameplayCueTags);
};
