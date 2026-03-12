// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GA_BasicAttack_Missile.generated.h"

class AMAProjectile;
/**
 * 
 */
UCLASS()
class UGA_BasicAttack_Missile : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	UAnimMontage* AttackMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<AMAProjectile> ProjectileClass;


	UFUNCTION()
	void StartShooting(FGameplayEventData Payload);
	UFUNCTION()
	void StopShooting(FGameplayEventData Payload);
	UFUNCTION()
	void ShootProjectile(FGameplayEventData Payload);
};
