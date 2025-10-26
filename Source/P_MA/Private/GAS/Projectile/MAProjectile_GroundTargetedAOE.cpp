// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAProjectile_GroundTargetedAOE.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "P_MA/P_MA.h"


AMAProjectile_GroundTargetedAOE::AMAProjectile_GroundTargetedAOE()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->ProjectileGravityScale = 1.f;
		ProjectileMovement->InitialSpeed = 0.f;
		ProjectileMovement->MaxSpeed = MaxSpeed;
		ProjectileMovement->bRotationFollowsVelocity = true;
	}
}


void AMAProjectile_GroundTargetedAOE::SetupCollision()
{
	if (!CollisionComponent) return;

	// 투사체 충돌 오브젝트 타입 생성하여 지정 - 스폰 시 서로 충돌 막기위함
	CollisionComponent->SetCollisionObjectType(ECC_Projectile);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 채널별 반응
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->OnComponentHit.AddDynamic(this, &AMAProjectile_GroundTargetedAOE::OnHitGround);

}

void AMAProjectile_GroundTargetedAOE::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeTime);
}

void AMAProjectile_GroundTargetedAOE::OnHitGround(UPrimitiveComponent* HitComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (HasAuthority() && !bHasExploded)
	{
		if (OtherActor == this || OtherActor == GetInstigator())
			return;
		
		if (OtherComp && (OtherComp->GetCollisionObjectType() == ECC_WorldStatic || OtherComp->GetCollisionObjectType() == ECC_WorldDynamic))
		{
			Explode(Hit);
			Destroy();
		}
	}
}

void AMAProjectile_GroundTargetedAOE::Explode(const FHitResult& Hit)
{
	if (bHasExploded)
		return;
	bHasExploded = true;
	
	TSubclassOf<UGameplayEffect> OriginalEffect = DamageGameplayEffect;
	DamageGameplayEffect = DamageEffect;
	ApplyAreaDamage(TargetImpactLocation, DamageRadius, Hit);
	DamageGameplayEffect = OriginalEffect;

	Multicast_PlayEffects(Hit.ImpactPoint);
}
