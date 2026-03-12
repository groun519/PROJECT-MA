// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "MAGameplayCue_CameraShake.generated.h"

/**
 * 
 */
UCLASS()
class UMAGameplayCue_CameraShake : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> RegularCameraShake;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCameraShakeBase> CriticalCameraShake;
};
