// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "P_MA/P_MA.h"

// Sets default values
AMAProjectile::AMAProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SetRootComponent(SphereComp);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	Niagara = CreateDefaultSubobject<UNiagaraComponent>("Niagara");
	Niagara->SetupAttachment(SphereComp);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	ProjectileMovement->UpdatedComponent = SphereComp;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
}

// Called when the game starts or when spawned
void AMAProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMAProjectile::OnOverlapBegin);
		SphereComp->OnComponentHit.AddDynamic(this, &AMAProjectile::OnHit);
	}
	SetLifeSpan(5.f);
}

void AMAProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())	return;

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams(ECC_Hitbox);
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ExplodeRadius);

	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape);
	DrawDebugSphere(GetWorld(), GetActorLocation(), ExplodeRadius, 12, FColor::Red, false, 2.0f);
	
	for (const FOverlapResult& OverlapResult : Overlaps)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (TargetActor && TargetActor != GetInstigator() && TargetActor != this)
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
			if (TargetASC)
			{
				if (DamageEffectSpecHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
				}
			}
		}
	}
	
	if (!bIsPenetrating)
	{
		Destroy();
	}
}

void AMAProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator()) return;

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (!SourceASC)
		return;

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjectQueryParams(ECC_Hitbox);
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ExplodeRadius);

	GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape);

	DrawDebugSphere(GetWorld(), Hit.ImpactPoint, ExplodeRadius, 12, FColor::Red, false, 2.0f);
	
	for (const FOverlapResult& OverlapResult : Overlaps)
	{
		AActor* TargetActor = OverlapResult.GetActor();
		if (TargetActor && TargetActor != GetInstigator())
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
			if (TargetASC)
			{
				if (DamageEffectSpecHandle.IsValid())
				{
					TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
				}
			}
		}
	}
	Destroy();
}

void AMAProjectile::InitializeProjectile(const FGameplayEffectSpecHandle& InSpecHandle, float InExplodeRadius, bool bInPenetrating)
{
	DamageEffectSpecHandle = InSpecHandle;
	ExplodeRadius = InExplodeRadius;
	bIsPenetrating = bInPenetrating;
}
