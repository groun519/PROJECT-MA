// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayAbility_Dash.generated.h"

/**
 * 
 */
UCLASS()
class UGameplayAbility_Dash : public UMAGameplayAbility
{
	GENERATED_BODY()

public:
	UGameplayAbility_Dash();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY()
	FVector DashStartDirection;
	// 서버에서 실제 대쉬 로직을 실행할 함수
	UFUNCTION(Server, Reliable)
	void Server_ExecuteDash(FGameplayEventData EventData);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* SkillMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Dash")
	float DashSpeed = 2000.0f;
};
