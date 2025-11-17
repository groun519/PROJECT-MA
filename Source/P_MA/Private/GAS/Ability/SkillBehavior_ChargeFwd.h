// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "GAS/Projectile/MATargetActor_ChargeAtFwd.h"
#include "SkillBehavior_ChargeFwd.generated.h"


class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class USkillBehavior_ChargeFwd : public UMASkillBehavior
{
	GENERATED_BODY()

public:
	virtual void OnActivate_Implementation() override;
	virtual void OnEndAbility_Implementation() override;

	virtual bool IsRequirePlayerInput() const override {return true;}
	virtual bool ShouldLockRotation() const override {return false;}
	virtual bool IsApplyCooldownImmediate() const override {return false;}

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMATargetActor_ChargeAtFwd> TargetActorClass;
	UPROPERTY()
	TObjectPtr<AMATargetActor_ChargeAtFwd> TargetActor;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxChargeDuration = 0.1f;
	UPROPERTY(EditDefaultsOnly)
	float SkillTimeoutDuration = 2.f;

	UPROPERTY(EditDefaultsOnly)
	float MinTraceDistance = 100.f;
	UPROPERTY(EditDefaultsOnly)
	float MaxTraceDistance = 1000.f;
	
	UPROPERTY(EditDefaultsOnly)
	float SkillWidth = 96.f;
	UPROPERTY()
	float DecalDepth = 10.f;
	
	UPROPERTY(EditDefaultsOnly)
	float VFXLength = 100.f;
	UPROPERTY(EditDefaultsOnly)
	float VFXWidth = 100.f;

	void SpawnVFX(float FinalLength);
	void CleanUp();
	
	TWeakObjectPtr<class UAbilityTask_WaitDelay> SkillTimeoutTask;
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> InputReleaseTask;
	
	UFUNCTION()
	void OnKeyReleased(float TimeHeld);
	UFUNCTION()
	void OnSkillTimeout();
};
