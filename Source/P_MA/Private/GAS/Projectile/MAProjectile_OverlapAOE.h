// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectileBase.h"
#include "MAProjectile_OverlapAOE.generated.h"

/**
 * 앞으로 날아가는 투사체 (기본적인 파이어볼 타입)
 */
UCLASS()
class AMAProjectile_OverlapAOE : public AMAProjectileBase
{
	GENERATED_BODY()

public:
	AMAProjectile_OverlapAOE();
	
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void ShootProjectile(float InSpeed, float InMaxDist, float InExplodeRange,
		FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle) override;

private:
	void Explode(FVector Location, const FHitResult& Hit);
	void TravelMaxDistanceReached();
};
