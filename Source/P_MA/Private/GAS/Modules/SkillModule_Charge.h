// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
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

	void StartWaitDamageEventTask(FName TagName);
	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);
	
	void StartWaitInputReleaseTask();
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	
	void StartMaxChargeDelayTask();
	UFUNCTION()
	void OnMaxCharged();

	void StartWaitTargetDataTask();
	void FinishTargetingTask();
	void DestroyActors();
private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ChargeStartEventTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> DamageEventTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> MaxChargeTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitTargetData> WaitTargetDataTask;

	bool bIsCharging = false;
	float FinalChargedDuration = 0.f;

	float CachedMaxChargeDuration = 3.f;
	float CachedMaxInputDelay = 3.5f;

	UPROPERTY()
	TObjectPtr<AGameplayAbilityTargetActor> CurrentTargetActor;
	UPROPERTY()
	TObjectPtr<class AMAAbilityRangeActor> SpawnedRangeActor;

};
