// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/OverlapProjectileActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"


AOverlapProjectileActor::AOverlapProjectileActor()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>("Root Component");
	SetRootComponent(RootComp);
	
	CollisionComp = CreateDefaultSubobject<USphereComponent>("CollisionComponent");
	CollisionComp->SetupAttachment(RootComp);

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>("NiagaraComponent");
	NiagaraComp->SetupAttachment(CollisionComp);
	
	bReplicates=true;
}

void AOverlapProjectileActor::BeginPlay()
{
	Super::BeginPlay();
}

void AOverlapProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorLocation(GetActorLocation() + MoveDir * DeltaTime * ProjectileSpeed);
}

void AOverlapProjectileActor::ShootProjectile(float InSpeed, float InMaxDist, FGenericTeamId InTeamId,
	FGameplayEffectSpecHandle InHitEffectHandle)
{
	ProjectileSpeed = InSpeed;
	HitEffectHandle = InHitEffectHandle;
	MoveDir = GetActorRotation().Vector();
	
	SetGenericTeamId(InTeamId);
	float TravelMaxTime = InMaxDist / InSpeed;
	GetWorld()->GetTimerManager().SetTimer(ShootTimerHandle, this, &AOverlapProjectileActor::TravelMaxDistanceReached, TravelMaxTime);
}

void AOverlapProjectileActor::SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult)
{
	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal;

	UAbilitySystemGlobals::Get().GetGameplayCueManager()->HandleGameplayCue(CueTargetActor, HitGameplayCueTag, EGameplayCueEvent::Executed,CueParams);
}


void AOverlapProjectileActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AOverlapProjectileActor, MoveDir);
	DOREPLIFETIME(AOverlapProjectileActor, TeamId);
	DOREPLIFETIME(AOverlapProjectileActor, ProjectileSpeed);
}

void AOverlapProjectileActor::NotifyActorBeginOverlap(class AActor* Other)
{
	if (!Other || Other == GetOwner())
		return;
	if (GetTeamAttitudeTowards(*Other) != ETeamAttitude::Hostile)
		return;

	UAbilitySystemComponent* OtherASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other);
	if (OtherASC)
	{
		if (HasAuthority() && HitEffectHandle.IsValid())
		{
			OtherASC->ApplyGameplayEffectSpecToSelf(*HitEffectHandle.Data.Get());
			GetWorldTimerManager().ClearTimer(ShootTimerHandle);
		}
		FHitResult HitResult;
		HitResult.ImpactPoint = GetActorLocation();
		HitResult.ImpactNormal = GetActorForwardVector();
		SendLocalGameplayCue(Other,HitResult);
		Destroy();
	}
}


void AOverlapProjectileActor::TravelMaxDistanceReached()
{
	Destroy();
}
