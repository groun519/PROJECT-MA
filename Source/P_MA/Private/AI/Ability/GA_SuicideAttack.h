// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GA_SuicideAttack.generated.h"

/**
 * 
 */
UCLASS()
class UGA_SuicideAttack : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION()
	void OnRandomDelayFinished();

	UFUNCTION()
	void OnDamageEvent(FGameplayEventData Data);

	UFUNCTION()
	void OnEndEventReceived(FGameplayEventData Data);
	
private:
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* SuicideMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	UPROPERTY()
	TArray<AActor*> IgnoreTargets;
};
