// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "SkillBehavior_ChargeTarget.generated.h"

class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class USkillBehavior_ChargeTarget : public UMASkillBehavior
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
	TSubclassOf<AMAAbilityRangeActor> MaxDistanceActorClass;
	UPROPERTY()
	TObjectPtr<AMAAbilityRangeActor> DistanceActor;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMATargetActor_ChargeAtTarget> TargetActorClass;
	UPROPERTY()
	TObjectPtr<class AMATargetActor_ChargeAtTarget> TargetActor;

	UPROPERTY(EditDefaultsOnly)
	float MaxDistance = 1000.f;
	UPROPERTY(EditDefaultsOnly)
	float MaxSize = 500.f;
	UPROPERTY(EditDefaultsOnly)
	float MinSize = 50.f;
	UPROPERTY(EditDefaultsOnly)
	float MaxHoldDuration = 3.f;
	UPROPERTY(EditDefaultsOnly)
	float TimeoutDuration = 4.5f;

	UPROPERTY(EditDefaultsOnly)
	float VFXRadius = 100.f;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ExecutionVFX;

	TWeakObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetData;
	TWeakObjectPtr<class UAbilityTask_WaitDelay> WaitDelay;
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> WaitInputRelease;
	
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void OnDelayFinished();
	UFUNCTION()
	void OnReleased(float TimeHeld);
	
	void SpawnVFX(FVector SpawnLoc, float FinalSize);
	void CleanUp();
	
	float PressedTime=0.f;
	float CachedChargeDuration;
};
