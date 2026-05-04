#pragma once

#include "CoreMinimal.h"

class AActor;
class UMASkillAbility;
class UAbilitySystemComponent;
enum class EMASkillStatusEffectSourceType : uint8;
struct FGameplayEffectSpecHandle;
struct FHitResult;
struct FResolvedSkillHitEffects;
struct FResolvedStatusEffect;

class P_MA_API MASkillDamageApplicator final
{
public:
	struct FMASkillDamageApplicationContext
	{
		AActor* InstigatorActor = nullptr;
		AActor* EffectCauser = nullptr;
		FVector StatusEffectSourcePoint = FVector::ZeroVector;
	};

	static void ApplyHitResults(
		UMASkillAbility& OwnerAbility,
		const TArray<FHitResult>& HitResults,
		const FResolvedSkillHitEffects& ResolvedHitEffects,
		const FVector& StatusEffectSourcePoint);

	static void ApplyHitResult(
		UMASkillAbility& OwnerAbility,
		const FHitResult& HitResult,
		const FResolvedSkillHitEffects& ResolvedHitEffects,
		const FVector& StatusEffectSourcePoint);

	static void ApplyToTarget(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FResolvedSkillHitEffects& ResolvedHitEffects,
		const FMASkillDamageApplicationContext& ApplicationContext);

	static void ApplyToTarget(
		UAbilitySystemComponent& TargetASC,
		UMASkillAbility& OwnerAbility,
		const FHitResult& HitResult,
		const FResolvedSkillHitEffects& ResolvedHitEffects,
		const FVector& StatusEffectSourcePoint);

private:
	MASkillDamageApplicator() = delete;

	static FMASkillDamageApplicationContext MakeApplicationContext(
		const UMASkillAbility& OwnerAbility,
		const FVector& StatusEffectSourcePoint);

	static FVector ResolveStatusEffectSourcePoint(
		const FMASkillDamageApplicationContext& ApplicationContext,
		EMASkillStatusEffectSourceType SourceType);

	static bool ShouldApplyResolvedStatusEffect(
		UAbilitySystemComponent& TargetASC,
		AActor* TargetActor,
		const FResolvedStatusEffect& StatusEffect);

	static void ApplySpecToTargetASC(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FGameplayEffectSpecHandle& SpecHandle);

	static void ExecuteTargetGameplayCues(
		UAbilitySystemComponent& TargetASC,
		const FHitResult& HitResult,
		const FResolvedSkillHitEffects& ResolvedHitEffects,
		const FMASkillDamageApplicationContext& ApplicationContext);
};
