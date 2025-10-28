// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAProjectile_OverlapAOE.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMAProjectile_OverlapAOE::AMAProjectile_OverlapAOE()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InitSpeed;
		ProjectileMovement->MaxSpeed = InitSpeed;
		ProjectileMovement->ProjectileGravityScale=0.f;
	}
}

void AMAProjectile_OverlapAOE::SetupCollision()
{
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMAProjectile_OverlapAOE::OnOverlapPawn);
}

void AMAProjectile_OverlapAOE::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority() && !bHasExploded)
	{
		Explode(GetActorLocation(), FHitResult());
	}
	Super::EndPlay(EndPlayReason);
}

void AMAProjectile_OverlapAOE::OnOverlapPawn(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor==this || OtherActor==GetInstigator() || !Cast<APawn>(OtherActor))
		return;
	if (HasAuthority() && !bHasExploded)
	{
		Explode(SweepResult.ImpactPoint, SweepResult);
		Destroy();
	}
}

void AMAProjectile_OverlapAOE::Explode(FVector Location, const FHitResult& Hit)
{
	if (bHasExploded)
		return;
	bHasExploded = true;

	ApplyAreaDamage(Location, ImpactRadius, Hit);
	Multicast_PlayEffects(Location);
}
