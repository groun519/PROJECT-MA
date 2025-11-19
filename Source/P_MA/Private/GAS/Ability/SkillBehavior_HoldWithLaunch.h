// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/SkillBehavior_Hold.h"
#include "SkillBehavior_HoldWithLaunch.generated.h"

/**
 * 
 */
UCLASS()
class USkillBehavior_HoldWithLaunch : public USkillBehavior_Hold
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;

private:
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitLaunchTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitSmashTask;
	

	UFUNCTION()
	void StartLaunch(FGameplayEventData Payload);
	UFUNCTION()
	void StartSmash(FGameplayEventData Payload);
	
	FGameplayTag LaunchTag = FGameplayTag::RequestGameplayTag("Event.Montage.Damage");
	FGameplayTag SmashTag = FGameplayTag::RequestGameplayTag("Event.Montage.Launch");

	UPROPERTY(EditDefaultsOnly)
	float FirstLaunchSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly)
	float OtherLaunchSpeed = 500.f;
	UPROPERTY(EditDefaultsOnly)
	float SmashSpeed = 500.f;

	bool bHasLaunched = false;
	bool bHasSmashed = false;
};
