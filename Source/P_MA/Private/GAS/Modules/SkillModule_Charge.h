// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Modules/MASkillModule.h"
#include "SkillModule_Charge.generated.h"

/**
 * 
 */
UCLASS()
class USkillModule_Charge : public UMASkillModule
{
	GENERATED_BODY()

public:
	virtual void OnAbilityActivated() override;
	virtual void OnAbilityEnded(bool bWasCancelled) override;

protected:
	void StartMontageTask();
	UFUNCTION()
	void OnMontageEnded();

	void StartChargeTask();
	UFUNCTION()
	void OnChargeEventReceived(FGameplayEventData Payload);

	void StartWaitForEventTask(FName TagName);
	UFUNCTION()
	void OnEventReceived(FGameplayEventData Payload);
	
	void StartWaitInputReleaseTask();
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	
	void StartMaxChargeDelayTask();
	UFUNCTION()
	void OnMaxCharged();
private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ChargeStartEventTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> EventTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> MaxChargeTask;

	bool bIsCharging = false;
	float ChargeStartTime = 0.f;
	float FinalChargedDuration = 0.f;
};
