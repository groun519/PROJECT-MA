// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAProjectileBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "Engine/OverlapResult.h"
#include "Net/UnrealNetwork.h"

AMAProjectileBase::AMAProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	CollisionComp = CreateDefaultSubobject<USphereComponent>("CollisionComponent");
	SetRootComponent(CollisionComp);

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");
	NiagaraComp->SetupAttachment(CollisionComp);
	
	bReplicates=true;
}

void AMAProjectileBase::BeginPlay()
{
	Super::BeginPlay();
}

void AMAProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//bSweep은 루트컴포넌트의 충돌만 감지 -> Root가 SceneComponent면 충돌 무시
	SetActorLocation(GetActorLocation() + MoveDir * DeltaTime * ProjectileSpeed, true);
}

void AMAProjectileBase::ShootProjectile(float InSpeed, float InMaxDist, float InExplodeRange, FGenericTeamId InTeamId,
	FGameplayEffectSpecHandle InHitEffectHandle)
{
	ProjectileSpeed = InSpeed;
	HitEffectHandle = InHitEffectHandle;
	ExplodeRadius = InExplodeRange;
	MoveDir = GetActorRotation().Vector();
	
	SetGenericTeamId(InTeamId);
}

void AMAProjectileBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAProjectileBase, MoveDir);
	DOREPLIFETIME(AMAProjectileBase, TeamId);
	DOREPLIFETIME(AMAProjectileBase, ProjectileSpeed);
	DOREPLIFETIME(AMAProjectileBase, ExplodeRadius);
}

void AMAProjectileBase::ApplyAreaDamage(FVector OriginLocation, float DamageRadius, const FHitResult& Hit)
{
	if (!HasAuthority())
		return;
	if (!GetInstigator())
	{
		UE_LOG(LogTemp,Warning,TEXT("Not Instigator"));
	}
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (!SourceASC)
	{
		return;
	}
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
			if (TargetASC && AdditionalEffect)
			{
				FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
				if (Hit.IsValidBlockingHit())
					EffectContext.AddHitResult(Hit);
				FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(AdditionalEffect,1.f,EffectContext);
				if (SpecHandle.IsValid())
					SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
			}
		}
	}
}

void AMAProjectileBase::SendLocalGameplayCue(const FHitResult& Hit)
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (SourceASC)
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = Hit.ImpactPoint;
		CueParams.Normal = Hit.ImpactNormal;
		
		SourceASC->ExecuteGameplayCue(HitGameplayCueTag, CueParams);
	}
}