// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayAbility_Teleport.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_Teleport : public UMAGameplayAbility
{
	GENERATED_BODY()
public:
	UGameplayAbility_Teleport();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* SkillMontage;

	// 최대 텔레포트 거리
	UPROPERTY(EditDefaultsOnly, Category = "MA|Teleport")
	float MaxTeleportDistance = 1500.0f;

	// 텔레포트할 위치의 바닥을 찾기 위한 탐색 거리
	UPROPERTY(EditDefaultsOnly, Category = "MA|Teleport")
	float GroundTraceDistance = 100.0f;
	
	// 애니메이션에서 'Teleport.Start' 이벤트가 발생했을 때 호출될 함수
	UFUNCTION()
	void StartTeleporting(FGameplayEventData Data);

	UFUNCTION(Server, Reliable)
	void Server_ExecuteTeleport(FVector_NetQuantize TargetLocation);
};
