#pragma once

#include "CoreMinimal.h"

class AActor;
class UMASkillAbility;
class UMASkillModuleInstance;
class UAbilitySystemComponent;
enum class EMASkillStatusEffectSourceType : uint8;
struct FMADamageAppliedEvent;
struct FGameplayEffectSpecHandle;
struct FHitResult;
struct FResolvedSkillDamage;
struct FResolvedStatusEffect;

class P_MA_API MASkillDamageApplicator final
{
public:
	struct FMASkillDamageApplicationContext
	{
		AActor* InstigatorActor = nullptr;
		AActor* EffectCauser = nullptr;
		UMASkillModuleInstance* SkillEventScope = nullptr;
		FVector StatusEffectSourcePoint = FVector::ZeroVector;
	};

	static void ApplyHitResults(
		UMASkillAbility& OwnerAbility,
		UMASkillModuleInstance* SkillEventScope,
		const TArray<FHitResult>& HitResults,
		const FResolvedSkillDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

	static void ApplyHitResult(
		UMASkillAbility& OwnerAbility,
		UMASkillModuleInstance* SkillEventScope,
		const FHitResult& HitResult,
		const FResolvedSkillDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

	static void ApplyToTargetActor(
		UMASkillAbility& OwnerAbility,
		UMASkillModuleInstance* SkillEventScope,
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
		UMASkillModuleInstance* SkillEventScope,
		const FHitResult& HitResult,
		const FResolvedSkillDamage& ResolvedDamage,
		const FVector& StatusEffectSourcePoint);

private:
	MASkillDamageApplicator() = delete;

	static FMASkillDamageApplicationContext MakeApplicationContext(
		const UMASkillAbility& OwnerAbility,
		UMASkillModuleInstance* SkillEventScope,
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

	static void ApplySpecToTargetASC(
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
