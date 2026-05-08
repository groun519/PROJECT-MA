#include "GAS/Projectile/MAProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GenericTeamAgentInterface.h"
#include "Net/UnrealNetwork.h"
#include "P_MA/P_MA.h"

AMAProjectile::AMAProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.08f;
	SetActorTickEnabled(false);
	
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

	if (ProjectileParams.ContinuousHitSettings.bEnabled)
	{
		CheckContinuousHit();
	}

	if (bPendingDestroy) return;

	CheckAndHandleNearTargetHit();
	PreviousHitCheckLocation = GetActorLocation();
}

void AMAProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMAProjectile::OnOverlapBegin);
	}
	PreviousHitCheckLocation = GetActorLocation();
	ApplyProjectileVisuals();
	SetLifeSpan(5.f);
}

void AMAProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const FHitResult FinalHit = BuildHitResultFromOverlap(OtherActor, SweepResult, OtherComp);
	TryApplyHitToActor(OtherActor, FinalHit);
}

void AMAProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAProjectile, Rep_TrailVFX);
	DOREPLIFETIME(AMAProjectile, Rep_ElementalColor);
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

	const FMAProjectileContinuousHitSettings& ContinuousHitSettings = ProjectileParams.ContinuousHitSettings;
	PrimaryActorTick.TickInterval = FMath::Max(ContinuousHitSettings.TickInterval, 0.01f);
	PreviousHitCheckLocation = GetActorLocation();

	SetActorTickEnabled(
		ContinuousHitSettings.bEnabled
		|| (ProjectileParams.TargetingSettings.bHitOnlyDamageTarget
			&& ProjectileParams.TargetingSettings.DamageTarget.IsValid()));
}

FHitResult AMAProjectile::BuildHitResultFromOverlap(AActor* HitActor, const FHitResult& SweepResult, UPrimitiveComponent* OtherComp) const
{
	FHitResult FinalHit = SweepResult;
	if (FinalHit.bBlockingHit)
	{
		return FinalHit;
	}

	FVector ImpactPoint = GetActorLocation();
	FVector ImpactNormal = -GetActorForwardVector();

	if (OtherComp)
	{
		FVector ClosestPointOnEnemy;
		const float Distance = OtherComp->GetClosestPointOnCollision(GetActorLocation(), ClosestPointOnEnemy);
		if (Distance >= 0.f && !ClosestPointOnEnemy.IsZero())
		{
			ImpactPoint = ClosestPointOnEnemy;
			ImpactNormal = (GetActorLocation() - ClosestPointOnEnemy).GetSafeNormal(UE_SMALL_NUMBER, -GetActorForwardVector());
		}
	}

	if (!FinalHit.GetActor() && HitActor)
	{
		FinalHit = FHitResult(HitActor, OtherComp, ImpactPoint, ImpactNormal);
	}

	FinalHit.ImpactPoint = ImpactPoint;
	FinalHit.Location = ImpactPoint;
	FinalHit.ImpactNormal = ImpactNormal;
	FinalHit.Normal = ImpactNormal;
	return FinalHit;
}

FHitResult AMAProjectile::BuildHitResultFromActor(AActor* HitActor) const
{
	if (!HitActor)
	{
		return FHitResult(GetActorLocation(), GetActorLocation());
	}

	UPrimitiveComponent* HitComponent = Cast<UPrimitiveComponent>(HitActor->GetRootComponent());
	FVector ImpactPoint = HitActor->GetActorLocation();
	FVector ImpactNormal = (GetActorLocation() - ImpactPoint).GetSafeNormal(UE_SMALL_NUMBER, -GetActorForwardVector());

	if (HitComponent)
	{
		FVector ClosestPointOnTarget;
		const float Distance = HitComponent->GetClosestPointOnCollision(GetActorLocation(), ClosestPointOnTarget);
		if (Distance >= 0.f && !ClosestPointOnTarget.IsZero())
		{
			ImpactPoint = ClosestPointOnTarget;
			ImpactNormal = (GetActorLocation() - ClosestPointOnTarget).GetSafeNormal(UE_SMALL_NUMBER, -GetActorForwardVector());
		}
	}

	FHitResult HitResult(HitActor, HitComponent, ImpactPoint, ImpactNormal);
	HitResult.Location = ImpactPoint;
	HitResult.ImpactPoint = ImpactPoint;
	HitResult.Normal = ImpactNormal;
	HitResult.ImpactNormal = ImpactNormal;
	HitResult.TraceStart = GetActorLocation();
	HitResult.TraceEnd = ImpactPoint;
	return HitResult;
}

bool AMAProjectile::CanDamageActor(AActor* OtherActor) const
{
	if (!OtherActor) return false;

	const AActor* SourceActor = GetOwner() ? GetOwner() : GetInstigator();
	const IGenericTeamAgentInterface* SourceTeamInterface = Cast<IGenericTeamAgentInterface>(SourceActor);
	if (!SourceTeamInterface) return true;

	return MATargetRelation::MatchesMask(
		ProjectileParams.ResolvedHitEffects.TargetRelationMask,
		SourceTeamInterface->GetTeamAttitudeTowards(*OtherActor));
}

void AMAProjectile::ApplyResolvedHitEffectsToTarget(UAbilitySystemComponent* TargetASC, const FHitResult& HitResult)
{
	if (!TargetASC) return;

	MASkillDamageApplicator::FMASkillDamageApplicationContext ApplicationContext;
	ApplicationContext.InstigatorActor = GetInstigator() ? GetInstigator() : GetOwner();
	ApplicationContext.EffectCauser = this;
	ApplicationContext.StatusEffectSourcePoint = GetActorLocation();
	MASkillDamageApplicator::ApplyToTarget(*TargetASC, HitResult, ProjectileParams.ResolvedHitEffects, ApplicationContext);
}

bool AMAProjectile::TryApplyHitToActor(AActor* OtherActor, const FHitResult& HitResult)
{
	if (bPendingDestroy) return false;
	if (!OtherActor || OtherActor == this || OtherActor == GetInstigator() || OtherActor == GetOwner()) return false;

	const TWeakObjectPtr<AActor> HitActor = OtherActor;
	if (HitActors.Contains(HitActor)) return false;
	if (!CanDamageActor(OtherActor)) return false;

	if (ProjectileParams.TargetingSettings.bHitOnlyDamageTarget)
	{
		if (!ProjectileParams.TargetingSettings.DamageTarget.IsValid()
			|| OtherActor != ProjectileParams.TargetingSettings.DamageTarget.Get())
		{
			return false;
		}
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC) return false;

	ApplyResolvedHitEffectsToTarget(TargetASC, HitResult);

	HitActors.Add(HitActor);
	OnProjectileHit.Broadcast(OtherActor);

	const FMAProjectilePenetratingSettings& PenetratingSettings = ProjectileParams.PenetratingSettings;
	if (!PenetratingSettings.bIsPenetrating
		|| (PenetratingSettings.PenetratingCount > 0 && HitActors.Num() > PenetratingSettings.PenetratingCount))
	{
		BeginPendingDestroy();
	}

	return true;
}

void AMAProjectile::CheckContinuousHit()
{
	if (!SphereComp) return;

	const FVector CurrentLocation = GetActorLocation();
	const FVector PreviousLocation = PreviousHitCheckLocation;
	const float SweepDistance = FVector::Distance(PreviousLocation, CurrentLocation);

	const FMAProjectileContinuousHitSettings& ContinuousHitSettings = ProjectileParams.ContinuousHitSettings;
	if (SweepDistance < ContinuousHitSettings.MinSweepDistance)
	{
		return;
	}

	const int32 MaxSweepSubsteps = FMath::Max(ContinuousHitSettings.MaxSweepSubsteps, 1);
	const float MaxSweepSegmentLength = FMath::Max(ContinuousHitSettings.MaxSweepSegmentLength, 1.f);
	const int32 SweepSubsteps = FMath::Clamp(
		FMath::CeilToInt(SweepDistance / MaxSweepSegmentLength),
		1,
		MaxSweepSubsteps);

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Hitbox);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MAProjectileContinuousHit), false, this);
	QueryParams.AddIgnoredActor(this);
	if (AActor* InstigatorActor = GetInstigator())
	{
		QueryParams.AddIgnoredActor(InstigatorActor);
	}
	if (AActor* OwnerActor = GetOwner())
	{
		QueryParams.AddIgnoredActor(OwnerActor);
	}

	const FCollisionShape SweepShape = FCollisionShape::MakeSphere(SphereComp->GetScaledSphereRadius());
	UWorld* World = GetWorld();
	if (!World) return;

	for (int32 StepIndex = 0; StepIndex < SweepSubsteps && !bPendingDestroy; ++StepIndex)
	{
		const float SegmentStartAlpha = static_cast<float>(StepIndex) / SweepSubsteps;
		const float SegmentEndAlpha = static_cast<float>(StepIndex + 1) / SweepSubsteps;
		const FVector SegmentStart = FMath::Lerp(PreviousLocation, CurrentLocation, SegmentStartAlpha);
		const FVector SegmentEnd = FMath::Lerp(PreviousLocation, CurrentLocation, SegmentEndAlpha);

		TArray<FHitResult> SweepHits;
		if (!World->SweepMultiByObjectType(
			SweepHits,
			SegmentStart,
			SegmentEnd,
			FQuat::Identity,
			ObjectQueryParams,
			SweepShape,
			QueryParams))
		{
			continue;
		}

		SweepHits.Sort([](const FHitResult& Lhs, const FHitResult& Rhs)
		{
			return Lhs.Time < Rhs.Time;
		});

		for (const FHitResult& SweepHit : SweepHits)
		{
			if (TryApplyHitToActor(SweepHit.GetActor(), SweepHit) && bPendingDestroy)
			{
				return;
			}
		}
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
	ApplyPendingDestroyVisuals();

	if (HasAuthority())
	{
		MulticastBeginPendingDestroy();
		SetLifeSpan(0.5f);
	}
}

void AMAProjectile::ApplyPendingDestroyVisuals()
{
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
}

void AMAProjectile::MulticastBeginPendingDestroy_Implementation()
{
	if (bPendingDestroy) return;

	bPendingDestroy = true;
	ApplyPendingDestroyVisuals();
}

/** Targeting Logics **/
void AMAProjectile::CheckAndHandleNearTargetHit()
{
	const float NearTargetHitDistance = 25.f;

	if (!ProjectileParams.TargetingSettings.bHitOnlyDamageTarget) return;

	if (!ProjectileParams.TargetingSettings.DamageTarget.IsValid())
	{
		BeginPendingDestroy();
		return;
	}

	AActor* DamageTarget = ProjectileParams.TargetingSettings.DamageTarget.Get();
	if (HitActors.Contains(DamageTarget))
	{
		return;
	}

	const float DistanceSq = FVector::DistSquared(GetActorLocation(), DamageTarget->GetActorLocation());
	if (DistanceSq <= FMath::Square(NearTargetHitDistance))
	{
		TryApplyHitToActor(DamageTarget, BuildHitResultFromActor(DamageTarget));
	}
}
