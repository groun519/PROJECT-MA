// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MABaseProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/OverlapResult.h"

AMABaseProjectile::AMABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>("Collision Component");
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetCollisionProfileName("Projectile");
	CollisionComponent->SetIsReplicated(true);
	
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("Niagara Component");
	NiagaraComponent -> SetupAttachment(GetRootComponent());
	NiagaraComponent -> SetIsReplicated(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement");
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->SetIsReplicated(true);
}

void AMABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeTime);
	if (NiagaraComponent)
		NiagaraComponent->Activate();
	if (HasAuthority())
		SetupCollision();
}

void AMABaseProjectile::ApplyAreaDamage(FVector OriginLocation, float DamageRadius, const FHitResult& Hit)
{
	if (!HasAuthority() || !GetInstigator())
		return;

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (!SourceASC)
		return;

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams(ECC_Pawn);
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(DamageRadius);

	GetWorld()->OverlapMultiByObjectType(Overlaps, OriginLocation, FQuat::Identity, ObjectQueryParams, CollisionShape);
	for (const FOverlapResult& OverlapResult : Overlaps)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (TargetActor && TargetActor != GetInstigator())
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
			if (TargetASC && DamageGameplayEffect)
			{
				FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
				if (Hit.IsValidBlockingHit())
					EffectContext.AddHitResult(Hit);
				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGameplayEffect,1.f,EffectContext);
				if (SpecHandle.IsValid())
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

void AMABaseProjectile::Multicast_PlayEffects_Implementation(FVector Location)
{
	if (ImpactVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactVFX, Location);
	}
}


void AMABaseProjectile::SetupCollision()
{
}
