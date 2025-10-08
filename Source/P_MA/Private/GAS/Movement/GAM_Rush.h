// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
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

protected:
	UFUNCTION()
	void OnInputPressed(float TimePressed);

	UFUNCTION()
	void OnInputReleased(float TimePressed);

	UFUNCTION()
	void OnTimeout();

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> WaitInputReleaseTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> TimeoutTask;
	
	UPROPERTY(EditDefaultsOnly, Category="Rush")
	float MaxHoldDuration = 3.5f;

	void MontageToEndSection();

	bool bIsEnd = false;
};
