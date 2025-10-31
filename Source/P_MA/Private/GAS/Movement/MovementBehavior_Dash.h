// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "MovementBehavior_Dash.generated.h"

/**
 * 
 */
UCLASS()
class UMovementBehavior_Dash : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;


private:
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitDashStartEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitDamageTagEventTask;
	
	UFUNCTION()
	void OnDashStartEventReceived(FGameplayEventData Payload);
	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);
	
	FGameplayTag DashStartTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Dash.Start");
	
	
	UPROPERTY(EditDefaultsOnly)
	float UpLaunchForce = 100.f;
	UPROPERTY(EditDefaultsOnly)
	float ForwardLaunchForce = 100.f;
};
