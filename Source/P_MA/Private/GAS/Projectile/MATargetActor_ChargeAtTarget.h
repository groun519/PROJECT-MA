// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "MATargetActor_ChargeAtTarget.generated.h"

/**
 * 
 */
UCLASS()
class AMATargetActor_ChargeAtTarget : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AMATargetActor_ChargeAtTarget();

	virtual void Tick(float DeltaSeconds) override;
	virtual void ConfirmTargetingAndContinue() override;
	void Initialize(float InMaxDistance, float InMaxSize, float InMinSize, float InMaxHoldDuration);

	UPROPERTY(BlueprintReadOnly)
	FVector FinalImpactPoint;
	UPROPERTY(BlueprintReadOnly)
	float FinalChargeRatio;

private:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionComp;
	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* SkillRangeDecal;

	float StartTime;
	float MaxDistance;
	float MinSize;
	float MaxSize;
	float MaxHoldDuration;
	float CurrentSize;

	FVector GetTargetPoint() const;
	void HandleUpdate(float InElapsedTime);
};
