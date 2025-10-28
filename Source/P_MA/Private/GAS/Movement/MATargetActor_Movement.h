// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "MATargetActor_Movement.generated.h"

/**
 * 
 */
UCLASS()
class AMATargetActor_Movement : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

private:
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;

	UPROPERTY()
	APlayerController* PlayerController;
};
