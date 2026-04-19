#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/Skill/CrowdControl/MASkillCrowdControlTypes.h"
#include "GAS/Skill/MASkillDamageConfig.h"
#include "GameplayEffectTypes.h"
#include "MASkillRuntimeContext.generated.h"

class AActor;
class UMASkillAbility;
struct FGameplayEventData;

USTRUCT()
struct FResolvedSkillHitEffects
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	int32 TargetRelationMask = MATargetRelation::ToMask(EMATargetRelation::None);

	UPROPERTY(Transient)
	FGameplayEffectSpecHandle DamageSpec;

	UPROPERTY(Transient)
	TArray<FResolvedCrowdControlEffect> CrowdControlEffects;
};

USTRUCT(BlueprintType)
struct FSkillRuntimeContext
{
	GENERATED_BODY()

	void Initialize(UMASkillAbility* InOwnerAbility);
	void Reset();
	void ClearDamageConfig();
	void AddDamageConfig(const FMASkillDamageConfig& DamageConfig);
	void MultiplyFinalDamageMultiplier(float Multiplier);
	void AddTargetRelationModifier(const FMASkillTargetRelationModifier& TargetRelationModifier);
	TArray<FHitResult> GetHitResultsFromPayload(const FGameplayEventData& Payload, int32 TargetRelationMask) const;
	FVector GetCrowdControlCenterPoint(const FGameplayEventData& Payload) const;
	FResolvedSkillHitEffects BuildResolvedHitEffects(const FMASkillDamageConfig& BaseDamageConfig) const;
	void ApplyResolvedHitEffectsToHitResult(const FHitResult& HitResult, const FResolvedSkillHitEffects& ResolvedHitEffects, const FVector& CenterSourcePoint) const;

	void ClearIgnoredActors()
	{
		IgnoredActors.Reset();
	}

	bool IsIgnoredActor(const AActor* Actor) const
	{
		return Actor && IgnoredActors.Contains(Actor);
	}

	void AddIgnoredActor(AActor* Actor)
	{
		if (Actor)
		{
			IgnoredActors.Add(Actor);
		}
	}

private:
	FMASkillDamageConfig BuildMergedDamageConfig(const FMASkillDamageConfig& BaseDamageConfig) const;
	int32 ResolveTargetRelationMask(int32 BaseRelationMask) const;
	FGameplayEffectSpecHandle MakeDamageSpec(const FMASkillDamageConfig& ResolvedDamageConfig) const;
	FVector ResolveCrowdControlSourcePoint(EMASkillCrowdControlSourceType SourceType, const FVector& CenterSourcePoint) const;
	bool ShouldApplyResolvedCrowdControlEffect(
		AActor* TargetActor,
		const FResolvedCrowdControlEffect& CrowdControlEffect) const;

	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerAbility = nullptr;

	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> IgnoredActors;

	// TODO: Future runtime modules should contribute into this shared damage config before actions resolve local damage settings.
	UPROPERTY(Transient)
	FMASkillDamageConfig AccumulatedDamageConfig;

	UPROPERTY(Transient)
	float AccumulatedFinalDamageMultiplier = 1.f;

	// TODO: Future runtime modules can push ordered target relation modifiers here after the action-local base mask is resolved.
	UPROPERTY(Transient)
	TArray<FMASkillTargetRelationModifier> AccumulatedTargetRelationModifiers;
};
