#include "GAS/Projectile/MAProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
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
	ProjectileMovement->bIsHomingProjectile = false;
}

void AMAProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasAuthority()) return;

	if (ProjectileParams.ContinuousHitSettings.bEnabled)
		CheckContinuousHit();

	if (bPendingDestroy) return;

	ApplyLaunchSpeedDecay(DeltaTime);
	PreviousHitCheckLocation = GetActorLocation();
}

void AMAProjectile::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMAProjectile::OnOverlapBegin);
	PreviousHitCheckLocation = GetActorLocation();
	ApplyProjectileVisuals();
	SetLifeSpan(5.f);
}

void AMAProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryApplyHitToActor(OtherActor, BuildHitResultFromOverlap(OtherActor, SweepResult, OtherComp));
}

void AMAProjectile::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAProjectile, bRep_HasElementalVisualData);
	DOREPLIFETIME(AMAProjectile, Rep_MainVFX);
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
	EventScopes = ProjectileParams.EventScopes;
	bRep_HasElementalVisualData = ProjectileParams.ElementalSettings.bHasElementalData;
	Rep_MainVFX = ProjectileParams.ElementalSettings.MainVFX;
	Rep_TrailVFX = ProjectileParams.ElementalSettings.TrailVFX;
	Rep_ElementalColor = ProjectileParams.ElementalSettings.ElementalColor;
	ApplyProjectileVisuals();
	BindHomingTarget();

	const FMAProjectileContinuousHitSettings& ContinuousHitSettings = ProjectileParams.ContinuousHitSettings;
	PrimaryActorTick.TickInterval = FMath::Max(ContinuousHitSettings.TickInterval, 0.01f);
	PreviousHitCheckLocation = GetActorLocation();

	SetActorTickEnabled(
		ContinuousHitSettings.bEnabled
		|| (ProjectileMovement->bIsHomingProjectile && bDecayLaunchSpeed));
}

FHitResult AMAProjectile::BuildHitResultFromOverlap(AActor* HitActor, const FHitResult& SweepResult, UPrimitiveComponent* OtherComp) const
{
	FHitResult FinalHit = SweepResult;
	if (FinalHit.bBlockingHit) return FinalHit;

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

bool AMAProjectile::CanDamageActor(AActor* OtherActor) const
{
	if (!OtherActor) return false;

	const AActor* SourceActor = GetOwner() ? GetOwner() : GetInstigator();
	if (MATargetRelation::IsSelfTarget(SourceActor, OtherActor))
	{
		return MATargetRelation::IncludesSelf(ProjectileParams.ResolvedDamage.TargetRelationMask);
	}

	const IGenericTeamAgentInterface* SourceTeamInterface = Cast<IGenericTeamAgentInterface>(SourceActor);
	if (!SourceTeamInterface) return true;

	return MATargetRelation::MatchesMask(
		ProjectileParams.ResolvedDamage.TargetRelationMask,
		SourceTeamInterface->GetTeamAttitudeTowards(*OtherActor));
}

void AMAProjectile::ApplyDamageToTarget(UAbilitySystemComponent* TargetASC, const FHitResult& HitResult)
{
	MASkillDamageApplicator::FMASkillDamageApplicationContext ApplicationContext;
	ApplicationContext.InstigatorActor = GetInstigator() ? GetInstigator() : GetOwner();
	ApplicationContext.EffectCauser = this;
	ApplicationContext.EventExecutorAbility = ProjectileParams.EventExecutorAbility.Get();
	ApplicationContext.EventScopes = EventScopes;
	ApplicationContext.StatusEffectSourcePoint = GetActorLocation();
	MASkillDamageApplicator::ApplyToTarget(*TargetASC, HitResult, ProjectileParams.ResolvedDamage, ApplicationContext);
}

bool AMAProjectile::TryApplyHitToActor(AActor* OtherActor, const FHitResult& HitResult)
{
	if (bPendingDestroy) return false;
	if (!OtherActor || OtherActor == this) return false;

	const TWeakObjectPtr<AActor> HitActor = OtherActor;
	if (HitActors.Contains(HitActor)) return false;
	if (!CanDamageActor(OtherActor)) return false;

	if (ProjectileParams.TargetSettings.bHitOnlyTarget)
	{
		if (OtherActor != ProjectileParams.TargetSettings.TargetActor.Get())
		{
			return false;
		}
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
	if (!TargetASC) return false;

	ApplyDamageToTarget(TargetASC, HitResult);

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
	const FVector CurrentLocation = GetActorLocation();
	const FVector PreviousLocation = PreviousHitCheckLocation;
	const float SweepDistance = FVector::Distance(PreviousLocation, CurrentLocation);

	const FMAProjectileContinuousHitSettings& ContinuousHitSettings = ProjectileParams.ContinuousHitSettings;
	if (SweepDistance < ContinuousHitSettings.MinSweepDistance) return;

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
	if (!MATargetRelation::IncludesSelf(ProjectileParams.ResolvedDamage.TargetRelationMask))
	{
		if (AActor* InstigatorActor = GetInstigator())
		{
			QueryParams.AddIgnoredActor(InstigatorActor);
		}
		if (AActor* OwnerActor = GetOwner())
		{
			QueryParams.AddIgnoredActor(OwnerActor);
		}
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
			if (TryApplyHitToActor(SweepHit.GetActor(), SweepHit) && bPendingDestroy) return;
		}
	}
}

void AMAProjectile::ApplyProjectileVisuals()
{
	const float SphereRadius = SphereComp->GetScaledSphereRadius();
	const auto ApplyVisual = [this, SphereRadius](
		UNiagaraComponent* NiagaraComponent,
		UNiagaraSystem* ElementalVFX,
		const FMAProjectileElementalVisualSettings& VisualSettings)
	{
		if (!NiagaraComponent) return;

		if (bRep_HasElementalVisualData && VisualSettings.bUseElementalVFX && ElementalVFX
			&& NiagaraComponent->GetAsset() != ElementalVFX)
		{
			NiagaraComponent->SetAsset(ElementalVFX);
			NiagaraComponent->ResetSystem();
		}

		if (bRep_HasElementalVisualData && VisualSettings.bUseElementalColor)
		{
			NiagaraComponent->SetVariableLinearColor(TEXT("User.BaseColor"), Rep_ElementalColor);
		}

		NiagaraComponent->SetVariableFloat(TEXT("User.Radius"), SphereRadius);
	};

	ApplyVisual(Niagara, Rep_MainVFX, MainVisualSettings);
	ApplyVisual(TrailNiagara, Rep_TrailVFX, TrailVisualSettings);
}

void AMAProjectile::BindHomingTarget()
{
	if (!ProjectileMovement->bIsHomingProjectile) return;

	AActor* TargetActor = ProjectileParams.TargetSettings.TargetActor.Get();
	USceneComponent* TargetComponent = TargetActor ? TargetActor->GetRootComponent() : nullptr;
	if (!TargetComponent)
	{
		ProjectileMovement->bIsHomingProjectile = false;
		return;
	}

	ProjectileMovement->HomingTargetComponent = TargetComponent;
	LaunchSpeed = ProjectileMovement->Velocity.Size();
	if (LaunchSpeed <= UE_SMALL_NUMBER)
	{
		LaunchSpeed = ProjectileMovement->InitialSpeed;
	}
	LaunchSpeedDecayElapsed = 0.f;
	bLaunchSpeedDecayFinished = false;
}

void AMAProjectile::ApplyLaunchSpeedDecay(float DeltaTime)
{
	if (!ProjectileMovement->bIsHomingProjectile || !bDecayLaunchSpeed || bLaunchSpeedDecayFinished) return;

	const float DecayDuration = FMath::Max(LaunchSpeedDecayDuration, 0.f);
	LaunchSpeedDecayElapsed = DecayDuration > 0.f
		? FMath::Min(LaunchSpeedDecayElapsed + DeltaTime, DecayDuration)
		: 0.f;

	const float Alpha = DecayDuration > 0.f ? LaunchSpeedDecayElapsed / DecayDuration : 1.f;
	const float TargetSpeed = LaunchSpeed * FMath::Lerp(1.f, LaunchSpeedEndScale, Alpha);
	const FVector CurrentDirection = ProjectileMovement->Velocity.GetSafeNormal();
	if (!CurrentDirection.IsNearlyZero())
	{
		ProjectileMovement->Velocity = CurrentDirection * TargetSpeed;
	}

	if (Alpha >= 1.f)
	{
		bLaunchSpeedDecayFinished = true;
		if (!ProjectileParams.ContinuousHitSettings.bEnabled)
		{
			SetActorTickEnabled(false);
		}
	}
}

void AMAProjectile::BeginPendingDestroy()
{
	if (bPendingDestroy) return;

	MulticastBeginPendingDestroy();
	SetLifeSpan(0.5f);
}

void AMAProjectile::ApplyPendingDestroyVisuals()
{
	Niagara->Deactivate();
	Niagara->SetVisibility(false, true);

	SphereComp->SetGenerateOverlapEvents(false);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();

	SetActorTickEnabled(false);
}

void AMAProjectile::MulticastBeginPendingDestroy_Implementation()
{
	if (bPendingDestroy) return;

	bPendingDestroy = true;
	ApplyPendingDestroyVisuals();
}
