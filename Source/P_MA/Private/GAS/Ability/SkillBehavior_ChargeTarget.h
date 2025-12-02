// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "SkillBehavior_ChargeTarget.generated.h"

struct FGameplayAbilityTargetDataHandle;
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
	virtual bool IsUseDamageNotify() const override {return false;}
	virtual bool IsApplyCooldownImmediate() const override {return false;}
	virtual float GetCurrentDamageMultiplier() const override;
	virtual void InitFromConfig(const FInstancedStruct& ConfigPayload) override;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMAAbilityRangeActor> MaxDistanceActorClass;
	UPROPERTY()
	TObjectPtr<AMAAbilityRangeActor> DistanceActor;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMATargetActor_ChargeAtTarget> TargetActorClass;
	UPROPERTY()
	TObjectPtr<class AMATargetActor_ChargeAtTarget> TargetActor;
	
	float MaxChargeDuration;
	float MaxDistance;
	float MaxSize;
	float MinSize;
	float VFXRadius=200.f;
	
	TWeakObjectPtr<class UAbilityTask_WaitTargetData> WaitTargetData;
	TWeakObjectPtr<class UAbilityTask_WaitDelay> WaitDelay;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitSlowTagTask;
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> WaitInputRelease;

	FGameplayTag ChargeStartTag = FGameplayTag::RequestGameplayTag("Event.Montage.SlowPlay");
	
	UFUNCTION()
	void TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void TargetCancelled(const FGameplayAbilityTargetDataHandle& Data);
	UFUNCTION()
	void OnDelayFinished();
	UFUNCTION()
	void OnReleased(float TimeHeld);
	UFUNCTION()
	void OnChargeEventReceived(FGameplayEventData Payload);
	
	void SpawnVFX(FVector SpawnLoc, float FinalSize);
	void CleanUp();
	
	float PressedTime=0.f;
	float CachedChargeDuration;
};
