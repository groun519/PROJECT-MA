// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "P_MA/P_MA.h"


AMAProjectile::AMAProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SetRootComponent(SphereComp);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);

	Niagara = CreateDefaultSubobject<UNiagaraComponent>("Niagara");
	Niagara->SetupAttachment(SphereComp);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementComponent");
	ProjectileMovement->UpdatedComponent = SphereComp;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
}


void AMAProjectile::SetGameplayCueTag(FGameplayTag Tag)
{
	if (Tag.IsValid())
	{
		HitGameplayCueTag = Tag;
	}
}

void AMAProjectile::SetProjectileVFX(UNiagaraSystem* NewVFX)
{
	if (HasAuthority())
	{
		Rep_ProjectileVFX = NewVFX;
		OnRep_ProjectileVFX();
	}
}

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
	if (HitActors.Contains(OtherActor)) return;
	if (OnlyDamageTarget.IsValid() && OtherActor != OnlyDamageTarget.Get()) return;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC && DamageEffectSpecHandle.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
	}
	HitActors.Add(OtherActor);

	if (ExplodeRadius > 0.f)
	{
		TArray<FOverlapResult> Overlaps;
		FCollisionObjectQueryParams ObjectQueryParams(ECC_Hitbox);
		FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ExplodeRadius);
		
		GetWorld()->OverlapMultiByObjectType(Overlaps, GetActorLocation(), FQuat::Identity, ObjectQueryParams, CollisionShape);
		DrawDebugSphere(GetWorld(), GetActorLocation(), ExplodeRadius, 12, FColor::Red, false, 2.0f);

		for (const FOverlapResult& OverlapResult : Overlaps)
		{
			AActor* AoE_Target = OverlapResult.GetActor();
			if (AoE_Target && AoE_Target != GetInstigator() && AoE_Target != this && !HitActors.Contains(AoE_Target))
			{
				UAbilitySystemComponent* AoE_ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AoE_Target);
				if (AoE_ASC && DamageEffectSpecHandle.IsValid())
				{
					AoE_ASC->ApplyGameplayEffectSpecToSelf(*DamageEffectSpecHandle.Data.Get());
					HitActors.Add(AoE_Target);
				}
			}
		}
	}

	
	if (HitGameplayCueTag.IsValid())
	{
		FHitResult FinalHit = SweepResult;
		if (!FinalHit.bBlockingHit)
		{
			FVector ImpactPoint = GetActorLocation();
			FVector ImpactNormal = -GetActorForwardVector();

			if (OtherComp)
			{
				FVector ClosestPointOnEnemy;
				float Distance = OtherComp->GetClosestPointOnCollision(GetActorLocation(), ClosestPointOnEnemy);
				if (Distance >= 0.f && !ClosestPointOnEnemy.IsZero())
				{
					ImpactPoint = ClosestPointOnEnemy;
					ImpactNormal = (GetActorLocation() - ClosestPointOnEnemy).GetSafeNormal();
				}
			}
			FinalHit.ImpactPoint = ImpactPoint;
			FinalHit.ImpactNormal = ImpactNormal;
		}
		SendLocalGameplayCue(FinalHit);
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
	if (OnlyDamageTarget.IsValid() && OtherActor != OnlyDamageTarget.Get()) return;

	if (HitGameplayCueTag.IsValid())
	{
		SendLocalGameplayCue(Hit);
	}
		
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
			if (OnlyDamageTarget.IsValid() && TargetActor != OnlyDamageTarget.Get()) continue;
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

void AMAProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAProjectile, Rep_ProjectileVFX);
}

void AMAProjectile::SendLocalGameplayCue(const FHitResult& HitResult)
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (SourceASC)
	{
		FGameplayCueParameters CueParams;
		CueParams.Location=HitResult.ImpactPoint;
		CueParams.Normal = HitResult.ImpactNormal;

		SourceASC->ExecuteGameplayCue(HitGameplayCueTag, CueParams);
	}
}

void AMAProjectile::OnRep_ProjectileVFX()
{
	if (Niagara && Rep_ProjectileVFX)
	{
		Niagara->SetAsset(Rep_ProjectileVFX);
		Niagara->ResetSystem();
	}
}

void AMAProjectile::InitializeProjectile(const FGameplayEffectSpecHandle& InSpecHandle, float InExplodeRadius, bool bInPenetrating)
{
	DamageEffectSpecHandle = InSpecHandle;
	ExplodeRadius = InExplodeRadius;
	bIsPenetrating = bInPenetrating;
}
