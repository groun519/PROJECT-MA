#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"

class UAbilitySystemComponent;
class UMASkillAbility;
struct FGameplayEffectContextHandle;
struct FMASkillScopes;
struct FMASkillPayloadStore;
struct FMASkillPayloadAccess;

class P_MA_API MASkillDamageResolver final
{
public:
	static FMAResolvedDamage Resolve(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& Scopes,
		const FMASkillDamageConfig& DamageConfig,
		const FMASkillPayloadStore& PayloadStore);
	static FMAResolvedDamage Resolve(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& Scopes,
		const FMASkillDamageConfig& DamageConfig,
		const FMASkillPayloadAccess& Payloads);
	static FMAResolvedDamage Resolve(
		UAbilitySystemComponent& SourceASC,
		const FGameplayEffectContextHandle& SourceContext,
		const FMASkillDamageConfig& DamageConfig);

private:
	MASkillDamageResolver() = delete;

	static void ApplyDamageOverTimeConfig(FGameplayEffectSpecHandle& SpecHandle, const FMASkillDamageOverTimeConfig& DamageOverTime);
	static FMADamageExecutionConfig ResolveExecutionConfig(const FMASkillDamageConfig& DamageConfig, const FMASkillPayloadStore& PayloadStore);
	static FMADamageExecutionConfig ResolveExecutionConfig(const FMASkillDamageConfig& DamageConfig, const FMASkillPayloadAccess& Payloads);
	static FMADamageExecutionConfig ScaleDamageConfigForTick(const FMADamageExecutionConfig& DamageConfig, int32 TickCount);
	static FGameplayEffectSpecHandle MakeDamageEffectSpec(
		UAbilitySystemComponent& SourceASC,
		UMASkillAbility* OwnerAbility,
		const FGameplayEffectContextHandle& SourceContext,
		bool bDamageOverTime,
		const FMADamageExecutionConfig& DamageConfig);
	static FGameplayEffectContextHandle MakeSkillEffectContext(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& Scopes);
	static void SetSpecContext(
		FGameplayEffectSpecHandle& SpecHandle,
		const FGameplayEffectContextHandle& SourceContext);
	static FMAResolvedDamage Resolve(
		UAbilitySystemComponent& SourceASC,
		UMASkillAbility* OwnerAbility,
		const FGameplayEffectContextHandle& SourceContext,
		const FMASkillDamageConfig& DamageConfig,
		const FMASkillPayloadAccess& Payloads);
	static void AppendElementalHitGameplayCueTag(
		const FGameplayTag& DamageTypeTag,
		FGameplayTagContainer& TargetGameplayCueTags);
};
