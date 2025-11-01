// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "MATargetActor_ImedDamageFwd.generated.h"

/**
 * 
 */
UCLASS()
class AMATargetActor_ImedDamageFwd : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AMATargetActor_ImedDamageFwd();
	
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION()
	void HandleChargeValueChanged(float NewChargeRatio);
private:
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* SkillDecal;

};
