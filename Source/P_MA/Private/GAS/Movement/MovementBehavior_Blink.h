// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "MovementBehavior_Blink.generated.h"

/**
 * WaitGameplayEvent는 즉시 서버에서 실행, 클라이언트의 마우스 위치 등 알 수 없음
 * WaitTargetData도 서버에서 시작, but 클라이언트에게 데이터를 받아오는 작업
 * 클라이언트가 데이터를 보내주면 서버에서 TargetConfirmed 델리게이트 실행
 */
UCLASS()
class UMovementBehavior_Blink : public UMASkillBehavior
{
	GENERATED_BODY()
	
public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
	virtual void InitFromConfig(const FInstancedStruct& ConfigPayload) override;
private:
	TWeakObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetDataTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitBlinkTagEventTask;
	
	UFUNCTION()
	void OnBlinkTagReceived(FGameplayEventData Payload);
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	
	FGameplayTag BlinkTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Teleport.Start");

	UPROPERTY()
	TSubclassOf<class AMATargetActor_Movement> TargetActorClass;
	
	UPROPERTY()
	float MaxBlinkDistance;
	
	/** true면 몽타주 태그가 이미 발동됨 */
	bool bBlinkTagReceived;
	/** true면 클라이언트가 마우스 위치를 보냈음 */
	bool bHasValidTargetLocation;
	/** 마우스 위치 캐시 */
	FVector CachedBlinkLocation;
	FRotator CachedBlinkRotation;

	void TryTeleport();
};
