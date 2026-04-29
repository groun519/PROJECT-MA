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

	TrailNiagara = CreateDefaultSubobject<UNiagaraComponent>("TrailNiagara");
	TrailNiagara->SetupAttachment(SphereComp);

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
	if (bPendingDestroy) return;
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator())	return;
	const TWeakObjectPtr<AActor> HitActor = OtherActor;
	if (HitActors.Contains(HitActor)) return;
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
	HitActors.Add(HitActor);
	OnProjectileHit.Broadcast(OtherActor);

	if (ProjectileParams.ElementalSettings.HitGameplayCueTag.IsValid())
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

		ExecuteHitGameplayCues(FinalHit);
	}
	
	const FMAProjectilePenetratingSettings& PenetratingSettings = ProjectileParams.PenetratingSettings;
	if (!PenetratingSettings.bIsPenetrating
		|| (PenetratingSettings.PenetratingCount > 0 && HitActors.Num() > PenetratingSettings.PenetratingCount))
	{
		BeginPendingDestroy();
	}
}

void AMAProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAProjectile, Rep_TrailVFX);
	DOREPLIFETIME(AMAProjectile, Rep_ElementalColor);
}

void AMAProjectile::SendLocalGameplayCue(const FGameplayTag& GameplayCueTag, const FHitResult& HitResult)
{
	if (!GameplayCueTag.IsValid()) return;

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetInstigator());
	if (!SourceASC) return;

	FGameplayCueParameters CueParams;
	CueParams.Location = HitResult.ImpactPoint;
	CueParams.Normal = HitResult.ImpactNormal;
	CueParams.RawMagnitude = 1.0f;

	SourceASC->ExecuteGameplayCue(GameplayCueTag, CueParams);
}

void AMAProjectile::ExecuteHitGameplayCues(const FHitResult& HitResult)
{
	if (ProjectileParams.ElementalSettings.HitGameplayCueTag.IsValid())
		SendLocalGameplayCue(ProjectileParams.ElementalSettings.HitGameplayCueTag, HitResult);
}

void AMAProjectile::OnRep_ProjectileVisuals()
{
	ApplyProjectileVisuals();
}

void AMAProjectile::InitializeProjectile(const FMAProjectileParams& InProjectileParams)
{
	ProjectileParams = InProjectileParams;
	Rep_TrailVFX = ProjectileParams.TrailVFX;
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

	for (const FResolvedStatusEffect& StatusEffect : ProjectileParams.StatusEffects)
	{
		if (!StatusEffect.SpecHandle.IsValid()) continue;

		FGameplayEffectSpecHandle StatusEffectSpecHandle = StatusEffect.SpecHandle;
		UMAAbilitySystemStatics::SetReactionSourcePoint(
			StatusEffectSpecHandle,
			ResolveStatusEffectSourcePoint(StatusEffect.SourceType));
		TargetASC->ApplyGameplayEffectSpecToSelf(*StatusEffectSpecHandle.Data.Get());
	}
}

void AMAProjectile::ApplyProjectileVisuals()
{
	const float SphereRadius = SphereComp ? SphereComp->GetScaledSphereRadius() : 0.f;

	if (Niagara)
	{
		Niagara->SetVariableLinearColor(TEXT("User.BaseColor"), Rep_ElementalColor);
		Niagara->SetVariableFloat(TEXT("User.Radius"), SphereRadius);
	}

	if (TrailNiagara)
	{
		if (TrailNiagara->GetAsset() != Rep_TrailVFX)
		{
			TrailNiagara->SetAsset(Rep_TrailVFX);
			TrailNiagara->ResetSystem();
		}

		TrailNiagara->SetVariableFloat(TEXT("User.Radius"), SphereRadius);
	}
}

void AMAProjectile::BeginPendingDestroy()
{
	if (bPendingDestroy) return;

	bPendingDestroy = true;

	if (Niagara)
	{
		Niagara->Deactivate();
		Niagara->SetVisibility(false, true);
	}

	if (SphereComp)
	{
		SphereComp->SetGenerateOverlapEvents(false);
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->StopMovementImmediately();
		ProjectileMovement->Deactivate();
	}

	SetActorTickEnabled(false);
	SetLifeSpan(0.5f);
}

FVector AMAProjectile::ResolveStatusEffectSourcePoint(EMASkillStatusEffectSourceType SourceType) const
{
	switch (SourceType)
	{
	case EMASkillStatusEffectSourceType::Center:
		return GetActorLocation();
	case EMASkillStatusEffectSourceType::Instigator:
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
		if (HitActors.Contains(ProjectileParams.TargetingSettings.DamageTarget))
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
		ExecuteHitGameplayCues(CueHitResult);
		BeginPendingDestroy();
	}
}
