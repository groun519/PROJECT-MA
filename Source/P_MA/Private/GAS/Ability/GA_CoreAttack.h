// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GA_CoreAttack.generated.h"

class AMonster;
class AMAProjectile;

UCLASS()
class UGA_CoreAttack : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_CoreAttack();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) const override;

	virtual const FGameplayTagContainer* GetCooldownTags() const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category="Attack")
	TSubclassOf<AMAProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	float SpawnDistanceFromCharacter = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	float ExplodeRadius = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	bool bIsPenetrating = false;

	UPROPERTY(EditDefaultsOnly, Category="Attack")
	float DamageMultiplier = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Attack|Homing")
	bool bUseHoming = true;

	UPROPERTY(EditDefaultsOnly, Category="Attack|Homing", meta=(ClampMin="0.0"))
	float HomingAccelerationMagnitude = 6000.f;

	UPROPERTY(EditDefaultsOnly, Category="Attack|Homing", meta=(ClampMin="0.0"))
	float HomingActivationDelay = 0.f;

	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float TargetingRange = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	bool bRequireTarget = true;

	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	bool bAimToTarget = true;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	float BaseCooldown = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	FGameplayTag CooldownTag;

	UPROPERTY(EditDefaultsOnly, Category="Cooldown")
	float MinAttackSpeedForCooldown = 0.1f;

private:
	AMonster* FindNearestMonster() const;
	void AimAtTarget(const AActor* Target) const;

	void SpawnProjectile();
	void ScheduleHomingAndCollision(AMAProjectile* Projectile);
	TSubclassOf<UGameplayEffect> GetBaseDamageEffect() const;
	FGameplayEffectSpecHandle MakeDamageSpec() const;
	TSubclassOf<UGameplayEffect> GetBaseCooldownEffect() const;

	UPROPERTY()
	TWeakObjectPtr<AActor> CachedTarget;
};
