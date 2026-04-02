#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GameplayEffectTypes.h"
#include "MASkillRuntimeContext.generated.h"

class AActor;
class AMAProjectile;
class UGameplayEffect;
class UMASkillAbility;
class UAnimInstance;
class UAnimMontage;
class UMASkillAction;
class USkeletalMeshComponent;
struct FGameplayEventData;
struct FMADamageExecutionConfig;

USTRUCT(BlueprintType)
struct FSkillRuntimeContext
{
	GENERATED_BODY()

	void Initialize(UMASkillAbility* InOwnerAbility);
	void Reset();
	void HandleEvent(const FGameplayEventData& Payload);
	void ClearDamageConfig();
	void AddDamageConfig(const FMADamageExecutionConfig& DamageConfig);
	TSet<FGameplayTag> ResolveRequiredEventTags() const;
	void ResolveActionsForEvent(const FGameplayEventData& Payload, TArray<UMASkillAction*>& OutActions) const;

	bool HasAuthority() const;
	AActor* GetAvatarActor() const;
	USkeletalMeshComponent* GetOwningMeshComponent() const;
	UWorld* GetWorld() const;
	UAnimInstance* GetOwnerAnimInstance() const;
	UAnimMontage* GetSkillMontage() const;
	bool TryGetCurrentSkillSection(UAnimInstance*& OutAnimInstance, UAnimMontage*& OutSkillMontage, FName& OutCurrentSectionName) const;
	TArray<FHitResult> GetHitResultsFromPayload(const FGameplayEventData& Payload) const;
	FGameplayEffectSpecHandle MakeDamageSpec(const FMADamageExecutionConfig* DamageConfig = nullptr) const;
	void ApplyDamageToHitResult(const FHitResult& HitResult, const FMADamageExecutionConfig* DamageConfig = nullptr) const;
	AMAProjectile* SpawnDamageProjectile(
		TSubclassOf<AMAProjectile> ProjectileClass,
		const FVector& SpawnLocation,
		const FRotator& SpawnRotation,
		const FMADamageExecutionConfig* DamageConfig = nullptr,
		float ExplodeRadius = 0.f,
		bool bIsPenetrating = false) const;

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
	FMADamageExecutionConfig BuildMergedDamageConfig(const FMADamageExecutionConfig* DamageConfig) const;
	void RefreshStateFromEvent(const FGameplayEventData& Payload);

	UPROPERTY(Transient)
	TObjectPtr<UMASkillAbility> OwnerAbility = nullptr;

	UPROPERTY(Transient)
	TSubclassOf<UGameplayEffect> ResolvedDamageEffect = nullptr;

	UPROPERTY(Transient)
	TSet<TObjectPtr<AActor>> IgnoredActors;

	// TODO: Future runtime modules should contribute into this shared damage config before actions resolve local damage settings.
	UPROPERTY(Transient)
	FMADamageExecutionConfig AccumulatedDamageConfig;
};
