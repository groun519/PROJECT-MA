// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAProjectile_GroundTargetedAOE.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"


AMAProjectile_GroundTargetedAOE::AMAProjectile_GroundTargetedAOE()
{
}


void AMAProjectile_GroundTargetedAOE::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		CollisionComp->OnComponentHit.AddDynamic(this, &AMAProjectile_GroundTargetedAOE::OnHit);
	}
}

void AMAProjectile_GroundTargetedAOE::ShootProjectile(float InSpeed, float InMaxDist, float InExplodeRange,
	FGenericTeamId InTeamId, FGameplayEffectSpecHandle InHitEffectHandle)
{
	Super::ShootProjectile(InSpeed, InMaxDist, InExplodeRange, InTeamId, InHitEffectHandle);
}

void AMAProjectile_GroundTargetedAOE::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                                            UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasExploded || OtherActor==this || OtherActor == GetInstigator())
		return;
	
	ApplyAreaDamage(GetActorLocation(), ExplodeRadius, Hit);
	SendLocalGameplayCue(Hit);
	Destroy();
}


