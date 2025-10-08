// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Movement/MAGameplayAbility_Movement.h"
#include "GAM_Dash.generated.h"

/**
 * 
 */
UCLASS()
class UGAM_Dash : public UMAGameplayAbility_Movement
{
	GENERATED_BODY()

public:
	UGAM_Dash();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
private:
	
	UPROPERTY(EditDefaultsOnly, Category="Dash")
	float UpLaunchForce = 100.f;
	
	UFUNCTION()
	void OnGameplayEventReceived(FGameplayEventData Data);
	
};
