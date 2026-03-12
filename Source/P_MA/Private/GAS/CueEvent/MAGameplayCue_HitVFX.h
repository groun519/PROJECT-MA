// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "MAGameplayCue_HitVFX.generated.h"

class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class UMAGameplayCue_HitVFX : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	UMAGameplayCue_HitVFX();

protected:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> HitNiagaraVFX;

	UPROPERTY(EditDefaultsOnly)
	float BaseVFXRadius = 300.f;
};
