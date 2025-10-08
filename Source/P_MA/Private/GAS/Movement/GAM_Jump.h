// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Movement/MAGameplayAbility_Movement.h"
#include "GAM_Jump.generated.h"

/**
 * 
 */
UCLASS()
class UGAM_Jump : public UMAGameplayAbility_Movement
{
	GENERATED_BODY()

public:
	UGAM_Jump();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
private:
	UFUNCTION()
	void OnStartTagReceived(FGameplayEventData Data);

	UFUNCTION()
	void OnEndTagReceived(FGameplayEventData Data);
	
	UFUNCTION()
	void ExecuteSlam();
	
	UPROPERTY(EditDefaultsOnly, Category="Jump")
	float ForwardLaunchForce = 1000.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Jump")
	float VerticalLaunchForce = 400.f;

	UPROPERTY(EditDefaultsOnly, Category="Slam")
	float SlamForce = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category="Slam")
	float SlamHopForce = 200.f;

	UPROPERTY(EditDefaultsOnly, Category="Slam")
	float SlamDelay = 0.2f;

	FVector TargetLocation;
};
