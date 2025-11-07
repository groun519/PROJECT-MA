// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GA_MonsterDash.generated.h"

/**
 * 
 */
UCLASS()
class UGA_MonsterDash : public UMAGameplayAbility
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
		bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnComboChangeEvent(FGameplayEventData Data);

	UFUNCTION()
	void OnDamageEvent(FGameplayEventData Data);

	UFUNCTION()
	void OnClearEvent(FGameplayEventData Data);

	UFUNCTION()
	void OnEndEventReceived(FGameplayEventData Data);
	
private:
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* DashMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	UPROPERTY()
	TArray<AActor*> IgnoreTargets;
};