// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GAS/Movement/MAGameplayAbility_Movement.h"
#include "GAM_Rush.generated.h"


/**
 * */
UCLASS()
class UGAM_Rush : public UMAGameplayAbility_Movement
{
	GENERATED_BODY()
	
public:
	UGAM_Rush();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	// 차지관련 : 어빌리티가 종료될 때 호출되는 함수를 오버라이드하여 UI 타이머를 정리합니다.
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

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

	// 차지 관련
	// 주기적으로 UI 업데이트 함수를 호출하기 위한 타이머 핸들
	FTimerHandle ChargeUpdateTimerHandle;

	// 홀드를 시작한 시간을 기록할 변수
	float StartTime = 0.f;

	// 타이머가 주기적으로 호출할 함수
	void UpdateChargeUI();
	// 여기까지
};