// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/MASkillBehavior.h"
#include "GAS/Projectile/MAAbilityRangeActor.h"
#include "GAS/Projectile/MATargetActor_ImedDamageFwd.h"
#include "SkillBehavior_ChargeFwd.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnChargeValueChanged, float /*NewChargeRatio*/);
class AGameplayAbilityTargetActor;
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

	FOnChargeValueChanged ChargeValueChanged;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMATargetActor_ImedDamageFwd> TargetActorClass;
	UPROPERTY()
	TObjectPtr<AMATargetActor_ImedDamageFwd> TargetActor;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMAAbilityRangeActor> RangeActorClass;
	UPROPERTY()
	TObjectPtr<AMAAbilityRangeActor> MaxRangeActor;
	UPROPERTY()
	TObjectPtr<AMAAbilityRangeActor> CurrentRangeActor;
	
	UPROPERTY(EditDefaultsOnly)
	float MaxChargeDuration = 0.1f;
	UPROPERTY(EditDefaultsOnly)
	float SkillTimeoutDuration = 2.f;

	UPROPERTY(EditDefaultsOnly)
	float MinTraceDistance = 100.f;
	UPROPERTY(EditDefaultsOnly)
	float MaxTraceDistance = 1000.f;
	
	TWeakObjectPtr<class UAbilityTask_WaitDelay> SkillTimeoutTask;
	TWeakObjectPtr<class UAbilityTask_WaitInputRelease> InputReleaseTask;

	UFUNCTION()
	void OnKeyReleased(float TimeHeld);
	UFUNCTION()
	void OnSkillTimeout();

	FTimerHandle ChargeUpdateHandle;
	float ChargeStartTime = 0.f;
	void ChargeUpdate();
};
