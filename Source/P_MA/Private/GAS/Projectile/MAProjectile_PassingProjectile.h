// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectile_OverlapAOE.h"
#include "MAProjectile_PassingProjectile.generated.h"

/**
 * 
 */
UCLASS()
class AMAProjectile_PassingProjectile : public AMAProjectile_OverlapAOE
{
	GENERATED_BODY()

	virtual void NotifyActorBeginOverlap(class AActor* OtherActor) override;
	virtual void ShootProjectile(float InSpeed, float InMaxDist, float InExplodeRange, FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle) override;
	void TravelMaxDistance();
};
