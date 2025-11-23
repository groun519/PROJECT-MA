// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GA_GiantSwing.generated.h"

/**
 * 
 */
UCLASS()
class UGA_GiantSwing : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnDamageEvent(FGameplayEventData Data);

	UFUNCTION()
	void OnEndEventReceived(FGameplayEventData Data);
	
	UFUNCTION()
	void OnGrabEvent(FGameplayEventData Data);
	
private:
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* GiantSwingMontage = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY()
	TArray<AActor*> IgnoreTargets;
	
	UPROPERTY(EditAnywhere, Category="Socket")
	FName GrabSocketName = "Hand_R_Grab";

	UPROPERTY()
	ACharacter* GrabbedTarget = nullptr;
};
