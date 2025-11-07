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
	UGA_MonsterDash();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
    	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnEndEventReceived(FGameplayEventData Data);

	UFUNCTION()
	void HitTarget(FGameplayEventData Data);

	static FGameplayTag GetTargetEventTag();
	
private:
	TSubclassOf<UGameplayEffect> GetDamageEffect() const;
	
	UPROPERTY()
	TArray<AActor*> IgnoreTargets;

	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* DashMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;
};
