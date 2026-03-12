// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GAS/Modules/MASkillModule.h"
#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "SkillModule_Instant.generated.h"

/**
 * 제일 기본적인 (즉발) 공격 로직
 */
UCLASS()
class USkillModule_Instant : public UMASkillModule
{
	GENERATED_BODY()

public:
	virtual void OnAbilityActivated() override;
	virtual void OnAbilityEnded(bool bWasCancelled) override;

protected:
	void StartMontageTask();
	UFUNCTION()
	void OnMontageEnded();

	void StartWaitDamageEventTask(FGameplayTag Tag);
	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);
	
	void StartWaitTargetDataTask();
	UFUNCTION()
	void OnTargetDataConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void OnTargetDataCancelled(const FGameplayAbilityTargetDataHandle& Data);
private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> DamageEventTask;
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitTargetData> WaitTargetDataTask;

	UPROPERTY()
	TObjectPtr<AMAAbilityRangeActor> SpawnedRangeActor;
	void DestroyRangeActor();

	FGameplayAbilityTargetDataHandle CachedTargetData;
};
