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
	virtual bool IsUseDamageNotify() const override {return false;}
	virtual bool IsApplyCooldownImmediate() const override {return false;}
	virtual float GetCurrentDamageMultiplier() const override;
	virtual void InitFromConfig(const FInstancedStruct& ConfigPayload) override;

private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMATargetActor_ChargeAtFwd> TargetActorClass;
	UPROPERTY()
	TObjectPtr<AMATargetActor_ChargeAtFwd> TargetActor;
	
	float MaxChargeDuration;
	float MinTraceDistance;
	float MaxTraceDistance;

	float SkillWidth = 96.f;
	float DecalDepth = 10.f;

	float VFXLength = 1000.f;
	float VFXWidth =120.f;

	float CachedChargeDuration;
	void SpawnVFX(float FinalLength);
	void CleanUp();
	
	TWeakObjectPtr<class UAbilityTask_WaitDelay> SkillTimeoutTask;
	TWeakObjectPtr<class UAbilityTask_WaitGameplayEvent> WaitSlowTagTask;
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> InputReleaseTask;

	FGameplayTag ChargeStartTag = FGameplayTag::RequestGameplayTag("Event.Montage.SlowPlay");
	
	UFUNCTION()
	void OnKeyReleased(float TimeHeld);
	UFUNCTION()
	void OnSkillTimeout();
	UFUNCTION()
	void OnChargeEventReceived(FGameplayEventData Payload);
};
