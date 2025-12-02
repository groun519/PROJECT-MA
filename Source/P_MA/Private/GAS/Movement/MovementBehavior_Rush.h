// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
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
	virtual bool IsApplyCooldownImmediate() const override {return false;}
	virtual void InitFromConfig(const FInstancedStruct& ConfigPayload) override;
private:
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> WaitInputRelease;
	TWeakObjectPtr<class UAbilityTask_WaitDelay> TimeoutTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitClearEventTask;

	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	UFUNCTION()
	void OnFinished();
	UFUNCTION()
	void ClearIgnore(FGameplayEventData Payload);
	
	float MaxRushDuration = 3.f;

	bool bIsEnd = false;
};
