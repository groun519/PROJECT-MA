// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectileBase.h"
#include "MAProjectile_GroundTargetedAOE.generated.h"

/**
 * 바닥 충돌형 투사체 (메테오 타입)
 */
UCLASS()
class AMAProjectile_GroundTargetedAOE : public AMAProjectileBase
{
	GENERATED_BODY()

public:
	AMAProjectile_GroundTargetedAOE();

	virtual void ShootProjectile(float InSpeed, float InMaxDist, float InExplodeRange,
		FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle) override;

protected:
	virtual void BeginPlay() override;
	
private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
