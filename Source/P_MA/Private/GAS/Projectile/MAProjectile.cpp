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
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f; // 틱 오버헤드 낮춤
	SetActorTickEnabled(false); // 기본적으로 false로 두어, 타게팅 투사체가 아니면 틱을 안 쓰게 함.
	
	bReplicates = true;
	SetReplicateMovement(true);

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

void AMAProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasAuthority()) return;

	CheckAndHandleNearTargetDestroy();
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
	if (bHitOnlyDamageTarget)
	{
		if (!DamageTarget.IsValid() || OtherActor != DamageTarget.Get()) return;
	}

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
	if (bHitOnlyDamageTarget)
	{
		if (!DamageTarget.IsValid() || OtherActor != DamageTarget.Get()) return;
	}

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
			if (bHitOnlyDamageTarget)
			{
				if (!DamageTarget.IsValid() || TargetActor != DamageTarget.Get()) continue;
			}
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

/** Targeting Logics **/
void AMAProjectile::CheckAndHandleNearTargetDestroy()
{
	const float NearTargetDestroyDistance = 25.f;

	if (!bHitOnlyDamageTarget) return;
	FHitResult CueHitResult;
	bool bShouldDestroy = false;

	if (!DamageTarget.IsValid())
	{
		CueHitResult.ImpactPoint = GetActorLocation();
		CueHitResult.ImpactNormal = -GetActorForwardVector();
		bShouldDestroy = true;
	}
	else
	{
		if (HitActors.Contains(DamageTarget.Get()))
		{
			SetActorTickEnabled(false);
			return;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), DamageTarget->GetActorLocation());
		if (DistanceSq <= FMath::Square(NearTargetDestroyDistance))
		{
			CueHitResult.ImpactPoint = DamageTarget->GetActorLocation();
			CueHitResult.ImpactNormal = (GetActorLocation() - DamageTarget->GetActorLocation()).GetSafeNormal();
			if (CueHitResult.ImpactNormal.IsNearlyZero())
			{
				CueHitResult.ImpactNormal = -GetActorForwardVector();
			}
			bShouldDestroy = true;
		}
	}

	if (bShouldDestroy)
	{
		if (HitGameplayCueTag.IsValid()) SendLocalGameplayCue(CueHitResult);
		Destroy();
	}
}

void AMAProjectile::SetDamageTarget(AActor* InTarget)
{
	DamageTarget = InTarget;
	bHitOnlyDamageTarget = DamageTarget.IsValid();
	SetActorTickEnabled(bHitOnlyDamageTarget && DamageTarget.IsValid());
}

void AMAProjectile::SetHitOnlyDamageTargetEnabled(bool bInEnabled)
{
	bHitOnlyDamageTarget = bInEnabled;
	SetActorTickEnabled(bHitOnlyDamageTarget && DamageTarget.IsValid());
}
/**/
