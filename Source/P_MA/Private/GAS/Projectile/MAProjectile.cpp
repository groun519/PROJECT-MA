// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Projectile/MAProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GenericTeamAgentInterface.h"
#include "Net/UnrealNetwork.h"
#include "P_MA/P_MA.h"

AMAProjectile::AMAProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.05f; // Lower tick overhead
	SetActorTickEnabled(false); // Set to false by default to prevent ticks from being used if it is not a targeting projectile.
	
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

void AMAProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMAProjectile::OnOverlapBegin);
	}
	ApplyProjectileVisuals();
	SetLifeSpan(5.f);
}

void AMAProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())	return;
	if (HitActors.Contains(OtherActor)) return;
	if (!CanDamageActor(OtherActor)) return;
	if (ProjectileParams.TargetingSettings.bHitOnlyDamageTarget)
	{
		if (!ProjectileParams.TargetingSettings.DamageTarget.IsValid()
			|| OtherActor != ProjectileParams.TargetingSettings.DamageTarget.Get())
		{
			return;
		}
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (TargetASC)
	{
		ApplyEffectSpecsToTarget(TargetASC);
	}
	HitActors.Add(OtherActor);
	OnProjectileHit.Broadcast(OtherActor);

	
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
	
	const FMAProjectilePenetratingSettings& PenetratingSettings = ProjectileParams.PenetratingSettings;
	if (!PenetratingSettings.bIsPenetrating
		|| (PenetratingSettings.PenetratingCount > 0 && HitActors.Num() > PenetratingSettings.PenetratingCount))
	{
		Destroy();
	}
}

void AMAProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAProjectile, Rep_ElementalColor);
}

void AMAProjectile::SendLocalGameplayCue(const FHitResult& HitResult)
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (!SourceASC) return;

	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal;
	CueParams.RawMagnitude = 1.0f;

	SourceASC->ExecuteGameplayCue(HitGameplayCueTag, CueParams);
}

void AMAProjectile::OnRep_ProjectileVisuals()
{
	ApplyProjectileVisuals();
}

void AMAProjectile::InitializeProjectile(const FMAProjectileParams& InProjectileParams)
{
	ProjectileParams = InProjectileParams;
	Rep_ElementalColor = ProjectileParams.ElementalSettings.ElementalColor;
	ApplyProjectileVisuals();
	SetActorTickEnabled(
		ProjectileParams.TargetingSettings.bHitOnlyDamageTarget
		&& ProjectileParams.TargetingSettings.DamageTarget.IsValid());
}

bool AMAProjectile::CanDamageActor(AActor* OtherActor) const
{
	if (!OtherActor) return false;

	const AActor* SourceActor = GetOwner() ? GetOwner() : GetInstigator();
	const IGenericTeamAgentInterface* SourceTeamInterface = Cast<IGenericTeamAgentInterface>(SourceActor);
	if (!SourceTeamInterface) return true;

	return MATargetRelation::MatchesMask(
		ProjectileParams.TargetRelationMask,
		SourceTeamInterface->GetTeamAttitudeTowards(*OtherActor));
}

void AMAProjectile::ApplyEffectSpecsToTarget(UAbilitySystemComponent* TargetASC)
{
	if (!TargetASC) return;

	if (ProjectileParams.DamageSpecHandle.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*ProjectileParams.DamageSpecHandle.Data.Get());
	}

	for (const FResolvedCrowdControlEffect& CrowdControlEffect : ProjectileParams.CrowdControlEffects)
	{
		if (!CrowdControlEffect.SpecHandle.IsValid()) continue;

		FGameplayEffectSpecHandle CrowdControlSpecHandle = CrowdControlEffect.SpecHandle;
		UMAAbilitySystemStatics::SetReactionSourcePoint(
			CrowdControlSpecHandle,
			ResolveCrowdControlSourcePoint(CrowdControlEffect.SourceType));
		TargetASC->ApplyGameplayEffectSpecToSelf(*CrowdControlSpecHandle.Data.Get());
	}
}

void AMAProjectile::ApplyProjectileVisuals()
{
	if (!Niagara) return;


	Niagara->SetVariableLinearColor(TEXT("User.BaseColor"), Rep_ElementalColor);
}

FVector AMAProjectile::ResolveCrowdControlSourcePoint(EMASkillCrowdControlSourceType SourceType) const
{
	switch (SourceType)
	{
	case EMASkillCrowdControlSourceType::Center:
		return GetActorLocation();
	case EMASkillCrowdControlSourceType::Instigator:
	default:
		if (const AActor* InstigatorActor = GetInstigator())
		{
			return InstigatorActor->GetActorLocation();
		}
		return GetActorLocation();
	}
}

/** Targeting Logics **/
void AMAProjectile::CheckAndHandleNearTargetDestroy()
{
	const float NearTargetDestroyDistance = 25.f;

	if (!ProjectileParams.TargetingSettings.bHitOnlyDamageTarget) return;
	FHitResult CueHitResult;
	bool bShouldDestroy = false;

	if (!ProjectileParams.TargetingSettings.DamageTarget.IsValid())
	{
		CueHitResult.ImpactPoint = GetActorLocation();
		CueHitResult.ImpactNormal = -GetActorForwardVector();
		bShouldDestroy = true;
	}
	else
	{
		if (HitActors.Contains(ProjectileParams.TargetingSettings.DamageTarget.Get()))
		{
			SetActorTickEnabled(false);
			return;
		}

		const float DistanceSq = FVector::DistSquared(
			GetActorLocation(),
			ProjectileParams.TargetingSettings.DamageTarget->GetActorLocation());
		if (DistanceSq <= FMath::Square(NearTargetDestroyDistance))
		{
			CueHitResult.ImpactPoint = ProjectileParams.TargetingSettings.DamageTarget->GetActorLocation();
			CueHitResult.ImpactNormal = (
				GetActorLocation() - ProjectileParams.TargetingSettings.DamageTarget->GetActorLocation()).GetSafeNormal();
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
