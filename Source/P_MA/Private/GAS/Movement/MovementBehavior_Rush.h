// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "MovementBehavior_Rush.generated.h"

/**
 * 
 */
UCLASS()
class UMovementBehavior_Rush : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;
	virtual bool IsRequirePlayerInput() const override {return true;}
	virtual bool ShouldLockRotation() const override {return false;}

private:
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> WaitInputRelease;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitDamageTagEventTask;
	TWeakObjectPtr<class UAbilityTask_WaitDelay> TimeoutTask;

	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	UFUNCTION()
	void OnFinished();
	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);

	FGameplayTag DamageEventTag = UMAAbilitySystemStatics::GetMontageDamageTag();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> MovementDamageEffect;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxRushDuration = 3.f;

	bool bIsEnd = false;
};
