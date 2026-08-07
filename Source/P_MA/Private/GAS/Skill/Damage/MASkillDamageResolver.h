#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"

class UMASkillAbility;
struct FMASkillPayloadStore;
struct FMASkillPayloadAccess;

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
		const FMASkillPayloadAccess& Payloads);

private:
	MASkillDamageResolver() = delete;

	static void ApplyDamageOverTimeConfig(FGameplayEffectSpecHandle& SpecHandle, const FMASkillDamageOverTimeConfig& DamageOverTime);
	static FMADamageExecutionConfig ResolveExecutionConfig(const FMASkillDamageConfig& DamageConfig, const FMASkillPayloadStore& PayloadStore);
	static FMADamageExecutionConfig ResolveExecutionConfig(const FMASkillDamageConfig& DamageConfig, const FMASkillPayloadAccess& Payloads);
	static FMADamageExecutionConfig ScaleDamageConfigForTick(const FMADamageExecutionConfig& DamageConfig, int32 TickCount);
	static void AppendElementalHitGameplayCueTag(
		const FGameplayTag& DamageTypeTag,
		FGameplayTagContainer& TargetGameplayCueTags);
};
