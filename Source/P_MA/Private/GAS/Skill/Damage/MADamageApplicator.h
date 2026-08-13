#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"

class AActor;
class UMASkillAbility;
class UAbilitySystemComponent;
enum class EMASkillStatusEffectSourceType : uint8;
struct FActiveGameplayEffectHandle;
struct FGameplayEffectContextHandle;
struct FGameplayEffectModCallbackData;
struct FGameplayEffectSpec;
struct FGameplayEffectSpecHandle;
struct FHitResult;
struct FMASkillDamageConfig;
struct FMASkillPayloadAccess;
struct FMASkillWorldAreaShape;
struct FMAResolvedDamage;
struct FResolvedStatusEffect;

class P_MA_API MADamageApplicator final
{
public:
	static void ApplyArea(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& EventScopes,
		const FMASkillWorldAreaShape& Area,
		const FMASkillDamageConfig& DamageConfig,
		const FMASkillPayloadAccess& Payloads);

	static void ApplyArea(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& EventScopes,
		const FMASkillWorldAreaShape& Area,
		TConstArrayView<FMASkillDamageConfig> DamageConfigs,
		const FMASkillPayloadAccess& Payloads);

	static void ApplyArea(
		const FGameplayEffectContextHandle& SourceContext,
		AActor& AreaOwner,
		const FMASkillWorldAreaShape& Area,
		TConstArrayView<FMASkillDamageConfig> DamageConfigs);

	static void ApplyHitResults(
		const TArray<FHitResult>& HitResults,
		const FMAResolvedDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

	static void ApplyToTargetActor(
		AActor& TargetActor,
		const FMAResolvedDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

	static void ApplyToTarget(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FMAResolvedDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

	static void PostProcessAppliedDamage(
		UAbilitySystemComponent& TargetASC,
		const FGameplayEffectModCallbackData& Data);

	static void NotifyTargetKilled(
		UAbilitySystemComponent& TargetASC,
		const FGameplayEffectSpec& KillingEffectSpec);

private:
	MADamageApplicator() = delete;

	static FVector ResolveStatusEffectSourcePoint(
		const FGameplayEffectContextHandle& ContextHandle,
		const FVector& StatusEffectSourcePoint,
		EMASkillStatusEffectSourceType SourceType);

	static bool ResolveSkillEventSource(
		const FGameplayEffectContextHandle& ContextHandle,
		UMASkillAbility*& OutAbility,
		FMASkillScopes& OutScopes);

	static void RegisterWithSkillRuntime(
		const FGameplayEffectContextHandle& ContextHandle,
		UAbilitySystemComponent& TargetASC,
		const FActiveGameplayEffectHandle& EffectHandle);

	static bool ShouldApplyResolvedStatusEffect(
		UAbilitySystemComponent& TargetASC,
		AActor* TargetActor,
		const FResolvedStatusEffect& StatusEffect);

	static FGameplayEffectSpecHandle MakeSpecWithHitResult(
		const FHitResult& HitResult,
		const FGameplayEffectSpecHandle& SpecHandle);

	static bool ApplyDamageSpecToTargetASC(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FMAResolvedDamage& ResolvedDamage,
		FGameplayEffectContextHandle& OutContextHandle);

	static void ApplyStatusEffects(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FMAResolvedDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

	static bool PostProcessDamage(
		UAbilitySystemComponent& TargetASC,
		const FGameplayEffectContextHandle& ContextHandle,
		const FHitResult& HitResult,
		AActor* TargetActor,
		const FGameplayTagContainer& TargetGameplayCueTags,
		FMASkillEvent& OutHitEvent);

	static bool ApplyToTargetInternal(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FMAResolvedDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint,
		FMASkillEvent& OutHitEvent);

	static void ExecuteTargetGameplayCues(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FGameplayTagContainer& GameplayCueTags,
		const FGameplayEffectContextHandle& ContextHandle);
};
