#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"

class UMASkillAbility;
struct FMASkillPayloadStore;
struct FMASkillPayloadAccessor;

class P_MA_API MASkillDamageResolver final
{
public:
	static FResolvedSkillDamage Resolve(UMASkillAbility& OwnerAbility, const FMASkillDamageConfig& DamageConfig);
	static FResolvedSkillDamage Resolve(
		UMASkillAbility& OwnerAbility,
		const FMASkillDamageConfig& DamageConfig,
		const FMASkillPayloadStore& PayloadStore);
	static FResolvedSkillDamage Resolve(
		UMASkillAbility& OwnerAbility,
		const FMASkillDamageConfig& DamageConfig,
		const FMASkillPayloadAccessor& Payloads);

private:
	MASkillDamageResolver() = delete;

	static void ApplyDamageOverTimeConfig(FGameplayEffectSpecHandle& SpecHandle, const FMASkillDamageOverTimeConfig& DamageOverTime);
	static FMADamageExecutionConfig ResolveExecutionConfig(const FMASkillDamageConfig& DamageConfig, const FMASkillPayloadStore& PayloadStore);
	static FMADamageExecutionConfig ResolveExecutionConfig(const FMASkillDamageConfig& DamageConfig, const FMASkillPayloadAccessor& Payloads);
	static FMADamageExecutionConfig ScaleDamageConfigForTick(const FMADamageExecutionConfig& DamageConfig, int32 TickCount);
	static void AppendElementalHitGameplayCueTag(UMASkillAbility& OwnerAbility, FGameplayTagContainer& TargetGameplayCueTags);
};
