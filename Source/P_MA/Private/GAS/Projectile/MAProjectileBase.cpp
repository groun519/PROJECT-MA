#include "GAS/Projectile/MAProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/DataTable.h"
#include "GAS/Projectile/MAProjectileMovementComponent.h"
#include "GAS/Skill/Area/Decal/MAAreaDecalData.h"
#include "GAS/Skill/Damage/MADamageApplicator.h"
#include "GenericTeamAgentInterface.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "P_MA/P_MA.h"
#include "Setting/MAGameSettings.h"

AMAProjectileBase::AMAProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.08f;
	SetActorTickEnabled(false);
	
	bReplicates = true;
	SetReplicateMovement(true);
	InitialLifeSpan = 5.f;

	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SetRootComponent(SphereComp);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);

	ProjectileDecal = CreateDefaultSubobject<UDecalComponent>("ProjectileDecal");
	ProjectileDecal->SetupAttachment(SphereComp);
	ProjectileDecal->SetRelativeLocation(FVector(0.f, 0.f, -100.f));
	ProjectileDecal->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	ProjectileDecal->SetRelativeScale3D(FVector::OneVector);
	ProjectileDecal->DecalSize = FVector(100.f, BaseRadius, BaseRadius);

	TrailNiagara = CreateDefaultSubobject<UNiagaraComponent>("TrailNiagara");
	TrailNiagara->SetupAttachment(SphereComp);

	ProjectileMovement = CreateDefaultSubobject<UMAProjectileMovementComponent>("ProjectileMovementComponent");
	ProjectileMovement->UpdatedComponent = SphereComp;
	ProjectileMovement->InitialSpeed = BaseSpeed;
	ProjectileMovement->MaxSpeed = BaseSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->bIsHomingProjectile = false;
}

void AMAProjectileBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SphereComp->SetSphereRadius(BaseRadius, false);
	ProjectileDecal->DecalSize = FVector(100.f, BaseRadius, BaseRadius);
}

void AMAProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!HasAuthority()) return;

	CheckContinuousSweepHit();
	PreviousHitCheckLocation = GetActorLocation();
}

void AMAProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeProjectileDecal();
	ApplyProjectileTrailVisuals();
	if (HasAuthority())
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AMAProjectileBase::OnOverlapBegin);
	PreviousHitCheckLocation = GetActorLocation();
}

void AMAProjectileBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	TryApplyHitToActor(OtherActor, BuildHitResultFromOverlap(OtherActor, SweepResult, OtherComp));
}

void AMAProjectileBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAProjectileBase, Rep_ProjectileRadius);
	DOREPLIFETIME(AMAProjectileBase, Rep_ElementalColor);
	DOREPLIFETIME(AMAProjectileBase, bRep_HasElementalVisualData);
	DOREPLIFETIME(AMAProjectileBase, Rep_TrailVFX);
}

void AMAProjectileBase::InitializeProjectile(const FMAProjectileParams& InProjectileParams)
{
	ProjectileParams = InProjectileParams;
	Rep_ProjectileRadius = BaseRadius * FMath::Max(ProjectileParams.ProjectileRadiusMultiplier, KINDA_SMALL_NUMBER);
	Rep_ElementalColor = ProjectileParams.ElementalSettings.ElementalColor;
	bRep_HasElementalVisualData = ProjectileParams.ElementalSettings.bHasElementalData;
	Rep_TrailVFX = ProjectileParams.ElementalSettings.TrailVFX;
	ApplyProjectileRadius();
	ApplyProjectileElementalColor();
	InitializeProjectileVisuals(ProjectileParams);

	const FMAProjectileContinuousHitSettings& ContinuousHitSettings = ProjectileParams.ContinuousHitSettings;
	PrimaryActorTick.TickInterval = FMath::Max(ContinuousHitSettings.TickInterval, 0.01f);
	ProjectileMovement->InitialSpeed = FMath::Max(BaseSpeed, KINDA_SMALL_NUMBER);
	SpeedMultiplier = 1.f;
	BaseHomingAccelerationMagnitude = ProjectileMovement->HomingAccelerationMagnitude;
	RefreshProjectileSpeed();
	BindHomingTarget();
	PreviousHitCheckLocation = GetActorLocation();
}

void AMAProjectileBase::SetProjectileSpeedMultiplier(float NewSpeedMultiplier)
{
	SpeedMultiplier = FMath::Max(NewSpeedMultiplier, KINDA_SMALL_NUMBER);
	RefreshProjectileSpeed();
}

void AMAProjectileBase::RefreshProjectileSpeed()
{
	// Override MaxSpeed
	const float CalcSpeed = FMath::Max(BaseSpeed * SpeedMultiplier, KINDA_SMALL_NUMBER);
	ProjectileMovement->MaxSpeed = CalcSpeed;

	// Override Projectile Vector
	const float CurSpeed = ProjectileMovement->Velocity.Size();
	const FVector CurDir = ProjectileMovement->Velocity.GetSafeNormal();
	if (CurSpeed < CalcSpeed && !CurDir.IsNearlyZero())
	{
		ProjectileMovement->Velocity = CurDir * CalcSpeed;
	}

	// Override HomingAccMag -> BaseHomingAccMag * SpeedMul^2.
	if (ProjectileMovement->bIsHomingProjectile)
	{
		// Preserve the configured turn radius as projectile speed scales.
		ProjectileMovement->HomingAccelerationMagnitude = BaseHomingAccelerationMagnitude * FMath::Square(SpeedMultiplier);
	}

	const float SweepDistancePerTick = CalcSpeed * PrimaryActorTick.TickInterval;
	SetActorTickEnabled(SweepDistancePerTick > SphereComp->GetScaledSphereRadius());
}

FHitResult AMAProjectileBase::BuildHitResultFromOverlap(AActor* HitActor, const FHitResult& SweepResult, UPrimitiveComponent* OtherComp) const
{
	FHitResult FinalHit = SweepResult;
	if (FinalHit.bBlockingHit) return FinalHit;

	FVector ImpactPoint = GetActorLocation();
	FVector ImpactNormal = -GetActorForwardVector();

	if (OtherComp)
	{
		FVector ClosestPointOnEnemy;
		const float Distance = OtherComp->GetClosestPointOnCollision(GetActorLocation(), ClosestPointOnEnemy);
		if (Distance >= 0.f)
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

bool AMAProjectileBase::CanDamageActor(AActor* OtherActor) const
{
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

void AMAProjectileBase::ApplyDamageToTarget(UAbilitySystemComponent* TargetASC, const FHitResult& HitResult)
{
	MADamageApplicator::ApplyToTarget(*TargetASC, HitResult, ProjectileParams.ResolvedDamage, GetActorLocation());
}

bool AMAProjectileBase::TryApplyHitToActor(AActor* OtherActor, const FHitResult& HitResult)
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
	if (ProjectileParams.MaxHitCount > 0 && HitActors.Num() >= ProjectileParams.MaxHitCount)
	{
		BeginPendingDestroy();
	}

	return true;
}

void AMAProjectileBase::CheckContinuousSweepHit()
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

// RepNotify로 사용중이어서 제거불가
void AMAProjectileBase::ApplyProjectileRadius()
{
	const float ProjectileRadius = FMath::Max(Rep_ProjectileRadius, 0.f);
	SphereComp->SetSphereRadius(ProjectileRadius, true);
	ProjectileDecal->DecalSize = FVector(100.f, ProjectileRadius, ProjectileRadius);
	ApplyProjectileTrailVisuals();
	OnProjectileRadiusChanged();
}

void AMAProjectileBase::InitializeProjectileDecal()
{
	if (const UDataTable* DecalDataTable = UMAGameSettings::Get()->GetAreaDecalDataTable())
	{
		if (const FMAAreaDecalDataRow* DecalRow = DecalDataTable->FindRow<FMAAreaDecalDataRow>(
			TEXT("Circle"),
			TEXT("ProjectileDecal")))
		{
			ProjectileDecal->SetDecalMaterial(DecalRow->DecalMaterial);
		}
	}
	ProjectileDecal->CreateDynamicMaterialInstance();
	ApplyProjectileElementalColor();
}

void AMAProjectileBase::ApplyProjectileElementalColor()
{
	if (UMaterialInstanceDynamic* DecalMID = Cast<UMaterialInstanceDynamic>(ProjectileDecal->GetDecalMaterial()))
	{
		DecalMID->SetVectorParameterValue(PARAM_AreaDecal_BaseColor, Rep_ElementalColor);
	}
	ApplyProjectileTrailVisuals();
	OnProjectileElementalColorChanged();
}

void AMAProjectileBase::ApplyProjectileTrailVisuals()
{
	if (bRep_HasElementalVisualData && TrailVisualSettings.bUseElementalVFX && Rep_TrailVFX
		&& TrailNiagara->GetAsset() != Rep_TrailVFX)
	{
		TrailNiagara->SetAsset(Rep_TrailVFX);
		TrailNiagara->ResetSystem();
	}

	if (bRep_HasElementalVisualData && TrailVisualSettings.bUseElementalColor)
	{
		TrailNiagara->SetVariableLinearColor(TEXT("User.BaseColor"), Rep_ElementalColor);
	}

	TrailNiagara->SetVariableFloat(TEXT("User.Radius"), SphereComp->GetScaledSphereRadius());
}

void AMAProjectileBase::BindHomingTarget()
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
}

void AMAProjectileBase::BeginPendingDestroy()
{
	if (bPendingDestroy) return;

	MulticastBeginPendingDestroy();
	if (PostHitVisualLifeSpan > 0.f)
	{
		SetLifeSpan(PostHitVisualLifeSpan);
	}
	else
	{
		Destroy();
	}
}

void AMAProjectileBase::ApplyPendingDestroyVisuals()
{
	SphereComp->SetGenerateOverlapEvents(false);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement->StopMovementImmediately();
	ProjectileMovement->Deactivate();

	SetActorTickEnabled(false);
	OnProjectilePendingDestroy();
}

void AMAProjectileBase::MulticastBeginPendingDestroy_Implementation()
{
	if (bPendingDestroy) return;

	bPendingDestroy = true;
	ApplyPendingDestroyVisuals();
}
