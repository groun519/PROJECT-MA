#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"

class AActor;
class UMASkillAbility;
class UMASkillModuleInstance;
class UAbilitySystemComponent;
enum class EMASkillStatusEffectSourceType : uint8;
struct FMADamageAppliedEvent;
struct FActiveGameplayEffectHandle;
struct FGameplayEffectSpecHandle;
struct FHitResult;
struct FMASkillDamageConfig;
struct FMASkillPayloadAccessor;
struct FMASkillWorldAreaShape;
struct FResolvedSkillDamage;
struct FResolvedStatusEffect;

class P_MA_API MASkillDamageApplicator final
{
public:
	struct FMASkillDamageApplicationContext
	{
		AActor* InstigatorActor = nullptr;
		AActor* EffectCauser = nullptr;
		UMASkillAbility* EventExecutorAbility = nullptr;
		FMASkillScopes EventScopes;
		FVector StatusEffectSourcePoint = FVector::ZeroVector;
	};

	static void ApplyArea(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& EventScopes,
		const FMASkillWorldAreaShape& Area,
		const FMASkillDamageConfig& DamageConfig,
		const FMASkillPayloadAccessor& Payloads);

	static void ApplyArea(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& EventScopes,
		const FMASkillWorldAreaShape& Area,
		TConstArrayView<FMASkillDamageConfig> DamageConfigs,
		const FMASkillPayloadAccessor& Payloads);

	static void ApplyHitResults(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& EventScopes,
		const TArray<FHitResult>& HitResults,
		const FResolvedSkillDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

	static void ApplyHitResult(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& EventScopes,
		const FHitResult& HitResult,
		const FResolvedSkillDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

	static void ApplyToTargetActor(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& EventScopes,
		AActor& TargetActor,
		const FResolvedSkillDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

	static void ApplyToTarget(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FResolvedSkillDamage& ResolvedDamage,
		const FMASkillDamageApplicationContext& ApplicationContext);

	static void ApplyToTarget(
		UAbilitySystemComponent& TargetASC,
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& EventScopes,
		const FHitResult& HitResult,
		const FResolvedSkillDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

private:
	MASkillDamageApplicator() = delete;

	static FMASkillDamageApplicationContext MakeApplicationContext(
		UMASkillAbility& OwnerAbility,
		const FMASkillScopes& EventScopes,
		const FVector& StatusEffectSourcePoint);

	static FVector ResolveStatusEffectSourcePoint(
		const FMASkillDamageApplicationContext& ApplicationContext,
		EMASkillStatusEffectSourceType SourceType);

	static bool ShouldApplyResolvedStatusEffect(
		UAbilitySystemComponent& TargetASC,
		AActor* TargetActor,
		const FResolvedStatusEffect& StatusEffect);

	static FGameplayEffectSpecHandle MakeSpecWithHitResult(
		const FHitResult& HitResult,
		const FGameplayEffectSpecHandle& SpecHandle);

	static FActiveGameplayEffectHandle ApplySpecToTargetASC(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FGameplayEffectSpecHandle& SpecHandle);

	static bool ApplyDamageSpecToTargetASC(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FGameplayEffectSpecHandle& SpecHandle,
		FMADamageAppliedEvent& OutDamageAppliedEvent);

	static void ExecuteTargetGameplayCues(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FResolvedSkillDamage& ResolvedDamage,
		const FMASkillDamageApplicationContext& ApplicationContext);
};
