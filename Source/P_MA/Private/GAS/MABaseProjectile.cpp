// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MABaseProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AMABaseProjectile::AMABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>("Collision Component");
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetIsReplicated(true);
	
	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>("Niagara Component");
	NiagaraComponent -> SetupAttachment(GetRootComponent());
	NiagaraComponent -> SetIsReplicated(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement");
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bIsHomingProjectile = false;
	ProjectileMovement->SetIsReplicated(true);
}

void AMABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeTime);

	if (bExplodeOnHit)
	{
		CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
		CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		CollisionComponent->OnComponentHit.AddDynamic(this, &AMABaseProjectile::OnCollisionHit);
	}
	else
	{
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMABaseProjectile::OnCollisionOverlap);
	}
	
}

void AMABaseProjectile::OnCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator() || OtherActor->IsA(AMABaseProjectile::StaticClass()))
		return;

	if (HasAuthority())
	{
		// 데미지 입힐 타겟 찾기 위한 충돌 지점
		TArray<FOverlapResult> OverlapResults;
		FCollisionObjectQueryParams ObjectQueryParams(ECC_Pawn);
		FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ImpactRadius);

		GetWorld()->OverlapMultiByObjectType(OverlapResults, SweepResult.ImpactPoint, FQuat::Identity, ObjectQueryParams, CollisionShape);
		// 스킬 시전자의 ASC 없으면 리턴
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
		if (!SourceASC)
		{
			Destroy();
			return;
		}
		
		Multicast_PlayOverlapEffects();
		
		// 오버랩된 유효 타겟에게 데미지 적용
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			AActor* TargetActor = OverlapResult.GetActor();
			if (TargetActor && TargetActor != GetInstigator())
			{
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
				if (TargetASC)
				{
					FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
					EffectContext.AddHitResult(SweepResult);
					FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGameplayEffect,1.f, EffectContext);

					if (SpecHandle.IsValid())
						SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
		}
		Destroy();
	}
}

void AMABaseProjectile::OnCollisionHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator() || OtherActor->IsA(AMABaseProjectile::StaticClass()))
		return;
		
	if (HasAuthority())
	{
		// 데미지 입힐 타겟 찾기 위한 충돌 지점 (Hit.ImpactPoint 사용)
		TArray<FOverlapResult> OverlapResults;
		FCollisionObjectQueryParams ObjectQueryParams(ECC_Pawn);
		FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ImpactRadius); // 스킬에서 설정된 ImpactRadius 사용

		GetWorld()->OverlapMultiByObjectType(OverlapResults, Hit.ImpactPoint, FQuat::Identity, ObjectQueryParams, CollisionShape);
		
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
		if (!SourceASC)
		{
			Destroy();
			return;
		}
		
		Multicast_PlayOverlapEffects();
		
		// 오버랩된 유효 타겟에게 데미지 적용
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			AActor* TargetActor = OverlapResult.GetActor();
			if (TargetActor && TargetActor != GetInstigator())
			{
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
				if (TargetASC)
				{
					FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
					EffectContext.AddHitResult(Hit);
					FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageGameplayEffect,1.f, EffectContext);

					if (SpecHandle.IsValid())
						SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
				}
			}
		}
		Destroy();
	}
}

void AMABaseProjectile::Multicast_PlayOverlapEffects_Implementation()
{
	if (ImpactVFX)
	{
		const FVector SpawnLocation = GetActorLocation();
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactVFX, SpawnLocation);
	}
}



