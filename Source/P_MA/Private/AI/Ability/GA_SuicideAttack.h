// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GA_SuicideAttack.generated.h"

UCLASS()
class UGA_SuicideAttack : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnDamageEvent(FGameplayEventData Data);

	UFUNCTION()
	void OnDistanceCheckTick();

private:
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* SuicideMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY()
	TArray<AActor*> IgnoreTargets;

	UPROPERTY(EditAnywhere, Category="Suicide")
	float TriggerRange = 400.f;

	UPROPERTY(EditAnywhere, Category="Suicide")
	float CheckInterval = 0.1f;

	UPROPERTY()
	bool bHasTriggeredExplosion = false;
};
