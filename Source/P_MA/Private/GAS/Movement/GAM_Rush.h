// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Movement/MAGameplayAbility_Movement.h"
#include "GAM_Rush.generated.h"


/**
 * 
 */
UCLASS()
class UGAM_Rush : public UMAGameplayAbility_Movement
{
	GENERATED_BODY()
	
public:
	UGAM_Rush();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void HandleInputReleased(float TimeWaited);
private:
	
	void WaitInputRelease();
};
