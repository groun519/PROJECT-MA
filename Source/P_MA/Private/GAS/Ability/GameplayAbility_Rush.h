// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayAbility_Rush.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_Rush : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Rush();
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//키 뗐을 때 호출될 함수
	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

protected:
	// 클라이언트의 요청을 받아 서버에서 실행될 함수
	UFUNCTION(Server, Reliable)
	void Server_EndRush();

	// 서버의 명령을 받아 모든 클라이언트에서 실행될 함수
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndRush();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* SkillMontage;

	// 적용할 '돌진 중' 상태 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "MA|Effect")
	TSubclassOf<UGameplayEffect> RushingEffectClass;

	UFUNCTION()
	void HandleEndEvent(FGameplayEventData Payload);
};
