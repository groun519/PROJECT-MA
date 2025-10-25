// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "MovementBehavior_Jump.generated.h"

/**
 * 
 */
UCLASS()
class UMovementBehavior_Jump : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;

private:
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitJumpStartEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitJumpEndEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitDamageTagEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitDelay> WaitSlamDelayTask;
	
	UFUNCTION()
	void OnJumpStartEventReceived(FGameplayEventData EventData);
	UFUNCTION()
	void OnJumpEndEventReceived(FGameplayEventData EventData);
	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData EventData);
	UFUNCTION()
	void ExecuteSlam();
	
	FGameplayTag JumpStartTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.Start");
	FGameplayTag JumpEndTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.End");
	FGameplayTag DamageEventTag = UMAAbilitySystemStatics::GetMontageDamageTag();

	UPROPERTY(EditDefaultsOnly)
	float MaxJumpDistance = 500.f;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> MovementDamageEffect;

	UPROPERTY(EditDefaultsOnly)
	float MaxJumpForce = 1000.f;
	UPROPERTY(EditDefaultsOnly)
	float MinJumpForce = 0.f;
	
	UPROPERTY(EditDefaultsOnly)
	float VerticalLaunchForce = 400.f;

	UPROPERTY(EditDefaultsOnly)
	float SlamForce = 2000.f;

	UPROPERTY(EditDefaultsOnly)
	float SlamHopForce = 200.f;

	UPROPERTY(EditDefaultsOnly)
	float SlamDelay = 0.2f;
};
