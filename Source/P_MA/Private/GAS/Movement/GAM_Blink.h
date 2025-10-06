// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Movement/MAGameplayAbility_Movement.h"
#include "GAM_Blink.generated.h"

/**
 * 
 */
UCLASS()
class UGAM_Blink : public UMAGameplayAbility_Movement
{
	GENERATED_BODY()

public:
	UGAM_Blink();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnBlinkEventReceived(FGameplayEventData Payload);
	
	UPROPERTY(EditDefaultsOnly, Category = "Blink")
	float BlinkDistance = 1000.0f;
};
