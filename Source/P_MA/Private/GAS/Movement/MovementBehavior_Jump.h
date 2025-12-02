// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "MovementBehavior_Jump.generated.h"

struct FGameplayAbilityTargetDataHandle;
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
	virtual void InitFromConfig(const FInstancedStruct& ConfigPayload) override;
	
private:
	TWeakObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetDataTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitJumpStartEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitJumpEndEventTask;

	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void OnJumpStartEventReceived(FGameplayEventData EventData);
	UFUNCTION()
	void OnJumpEndEventReceived(FGameplayEventData EventData);
	
	FGameplayTag JumpStartTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.Start");
	FGameplayTag JumpEndTag = FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.End");

	UPROPERTY()
	TSubclassOf<class AMATargetActor_Movement> TargetActorClass;
	
	float MaxJumpDistance;
	float MinJumpDistance;
	float MaxJumpForce;
	float MinJumpForce;
	float VerticalLaunchForce;
	float SlamForce;
	
	bool bJumpTagReceived;
	bool bHasValidTargetLocation;
	
	FVector CachedJumpLocation;

	void TryJump();
};
