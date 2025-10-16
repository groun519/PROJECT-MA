// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MABaseProjectile.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"

AMABaseProjectile::AMABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>("Collision Component");
	SetRootComponent(CollisionComponent);
	CollisionComponent->SetIsReplicated(true);

	ParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>("Particle Component");
	ParticleComponent->SetupAttachment(RootComponent);
	ParticleComponent->SetIsReplicated(true);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("Projectile Movement");
	ProjectileMovement->InitialSpeed = ProjectileSpeed;
	ProjectileMovement->MaxSpeed = ProjectileSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bIsHomingProjectile = false;
	ProjectileMovement->SetIsReplicated(true);
}

void AMABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(2.5f);
	if (HasAuthority())
	{//충돌 처리 (게임 로직) = 서버에서 실행
		CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AMABaseProjectile::OnCollisionOverlap);
	}
}

void AMABaseProjectile::OnCollisionOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor==this || OtherActor == GetInstigator())
		return;

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("[BaseProjectile] OnCollisionOverlap execute in Server"));
		// 데미지 입힐 타겟 찾기 위한 충돌 지점
		TArray<FOverlapResult> OverlapResults;
		FCollisionObjectQueryParams ObjectQueryParams(ECC_Pawn);
		FCollisionShape CollisionShape = FCollisionShape::MakeSphere(ImpactRadius);

		GetWorld()->OverlapMultiByObjectType(OverlapResults, SweepResult.ImpactPoint, FQuat::Identity, ObjectQueryParams, CollisionShape);
		// 스킬 시전자의 ASC 없으면 리턴
		UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
		if (!SourceASC)
		{
			UE_LOG(LogTemp,Warning, TEXT("[BaseProjectile] Instigator doesnt have ASC"));
			Destroy();
			return;
		}
		// 오버랩된 유효 타겟에게 데미지 적용
		for (const FOverlapResult& OverlapResult : OverlapResults)
		{
			AActor* TargetActor = OverlapResult.GetActor();
			if (TargetActor && TargetActor != GetInstigator())
			{
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
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



