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
	virtual float GetCurrentDamageMultiplier() const override;
	virtual void InitFromData(const FSkillDefinitionDT& Data) override;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMATargetActor_ChargeAtFwd> TargetActorClass;
	UPROPERTY()
	TObjectPtr<AMATargetActor_ChargeAtFwd> TargetActor;
	
	float MaxChargeDuration;
	float TimeoutDuration;
	float MinTraceDistance;
	float MaxTraceDistance;

	float SkillWidth;
	float DecalDepth = 10.f;

	float VFXLength;
	float VFXWidth;

	float CachedChargeDuration;
	void SpawnVFX(float FinalLength);
	void CleanUp();
	
	TWeakObjectPtr<class UAbilityTask_WaitDelay> SkillTimeoutTask;
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> InputReleaseTask;
	
	UFUNCTION()
	void OnKeyReleased(float TimeHeld);
	UFUNCTION()
	void OnSkillTimeout();
};
