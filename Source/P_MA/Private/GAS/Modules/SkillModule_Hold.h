// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Modules/MASkillModule.h"
#include "SkillModule_Hold.generated.h"

/**
 * 
 */
UCLASS()
class USkillModule_Hold : public UMASkillModule
{
	GENERATED_BODY()

public:
	virtual void OnAbilityActivated() override;
	virtual void OnAbilityEnded(bool bWasCancelled) override;

protected:
	void StartMontageTask();
	UFUNCTION()
	void OnMontageEnded();

	void StartWaitJumpSectionEventTask();
	UFUNCTION()
	void OnJumpSectionEventReceived(FGameplayEventData Payload);
	
	void StartWaitDamageEventTask(FGameplayTag EventTag);
	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);

	void StartWaitInputReleaseTask();
	UFUNCTION()
	void OnInputRelease(float TimeHeld);

	void StartMaxHoldDelayTask();
	UFUNCTION()
	void OnMaxHold();
	
private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> JumpMontageSectionTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> DamageEventTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitInputRelease> InputReleaseTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitDelay> MaxHoldTask;

	bool bIsHolding = false;
	float CachedHoldMultiplier = 0.8f;
	float CachedMaxHoldDuration = 2.5f;
};
