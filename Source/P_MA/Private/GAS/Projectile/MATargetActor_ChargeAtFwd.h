// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "MATargetActor_ChargeAtFwd.generated.h"

/**
 * 
 */
UCLASS()
class AMATargetActor_ChargeAtFwd : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AMATargetActor_ChargeAtFwd();
	
	virtual void Tick(float DeltaTime) override;
	
	void Initialize(float InMaxDistance, float InMinDistance, float InWidth, float InDepth, float InMaxChargeDuration);
	FGameplayAbilityTargetDataHandle GetTargetData();

private:
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* RootComp;
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* SkillDecal;
	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* MaxRangeDecal;
	UPROPERTY(VisibleAnywhere)
	class UDecalComponent* CurrentRangeDecal;

	float StartTime;
	float MinDistance;
	float MaxDistance;
	float SkillWidth;
	float DecalDepth;
	float MaxChargeDuration;

	void HandleUpdate(float InElapsedTime);
};
