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
	virtual void InitFromData(const FSkillDefinitionDT& Data) override;
private:
	TWeakObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetDataTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitJumpStartEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitJumpEndEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitDamageTagEventTask;

	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void OnJumpStartEventReceived(FGameplayEventData EventData);
	UFUNCTION()
	void OnJumpEndEventReceived(FGameplayEventData EventData);
	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData EventData);
	
	FGameplayTag JumpStartTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.Start");
	FGameplayTag JumpEndTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.End");

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMATargetActor_Movement> TargetActorClass;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxJumpDistance = 700.f;
	UPROPERTY(EditDefaultsOnly)
	float MinJumpDistance = 100.f;
	UPROPERTY(EditDefaultsOnly)
	float MaxJumpForce = 1000.f;
	UPROPERTY(EditDefaultsOnly)
	float MinJumpForce = 200.f;
	UPROPERTY(EditDefaultsOnly)
	float VerticalLaunchForce = 400.f;
	UPROPERTY(EditDefaultsOnly)
	float SlamForce = -2000.f;
	
	bool bJumpTagReceived;
	bool bHasValidTargetLocation;
	
	FVector CachedJumpLocation;

	void TryJump();
};
