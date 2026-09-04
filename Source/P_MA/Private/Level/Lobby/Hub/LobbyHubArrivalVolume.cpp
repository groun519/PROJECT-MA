#include "Level/Lobby/Hub/LobbyHubArrivalVolume.h"

#include "Components/SphereComponent.h"
#include "Level/Lobby/Hub/LobbyHubCharacter.h"
#include "P_MA/P_MA.h"

ALobbyHubArrivalVolume::ALobbyHubArrivalVolume()
{
	ArrivalArea = CreateDefaultSubobject<USphereComponent>(TEXT("ArrivalArea"));
	RootComponent = ArrivalArea;
	ArrivalArea->InitSphereRadius(750.f);
	ArrivalArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ALobbyHubArrivalVolume::Launch(
	ALobbyHubCharacter& Character,
	const FLobbyHubArrivalSpawn& ArrivalSpawn) const
{
	const float SafeMinLaunchSpeed = FMath::Max(MinLaunchSpeed, 0.f);
	const float SafeMaxLaunchSpeed = FMath::Max(MaxLaunchSpeed, SafeMinLaunchSpeed);
	const float SelectedLaunchSpeed = FMath::FRandRange(SafeMinLaunchSpeed, SafeMaxLaunchSpeed);
	const FVector InitialVelocity = FVector::UpVector * SelectedLaunchSpeed;
	ensureMsgf(
		Character.BeginArrival(InitialVelocity, ArrivalSpawn.GroundLocation),
		TEXT("Lobby Hub Character rejected its initial Arrival launch."));
}

bool ALobbyHubArrivalVolume::TryCreateArrivalSpawn(FLobbyHubArrivalSpawn& OutArrivalSpawn) const
{
	if (!ArrivalArea) return false;

	constexpr int32 MaxSelectionAttempts = 16;
	const FVector AreaCenter = ArrivalArea->GetComponentLocation();
	const float AreaRadius = FMath::Max(ArrivalArea->GetScaledSphereRadius(), 1.f);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LobbyHubArrivalGround), false, this);
	FCollisionObjectQueryParams GroundQueryParams;
	GroundQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
	FCollisionObjectQueryParams HitboxQueryParams;
	HitboxQueryParams.AddObjectTypesToQuery(ECC_Hitbox);

	for (int32 Attempt = 0; Attempt < MaxSelectionAttempts; ++Attempt)
	{
		const FVector2D Offset = FMath::RandPointInCircle(AreaRadius);
		const FVector CandidateLocation = AreaCenter + FVector(Offset.X, Offset.Y, 0.f);
		const FVector TraceStart = CandidateLocation + FVector::UpVector * AreaRadius;
		const FVector TraceEnd = CandidateLocation - FVector::UpVector * AreaRadius;
		FHitResult GroundHit;
		if (!GetWorld()->LineTraceSingleByObjectType(
			GroundHit,
			TraceStart,
			TraceEnd,
			GroundQueryParams,
			QueryParams))
		{
			continue;
		}
		if (FVector::DotProduct(GroundHit.ImpactNormal, FVector::UpVector) < 0.5f)
		{
			continue;
		}

		const FVector SpawnLocation = GroundHit.ImpactPoint
			+ FVector::UpVector * FMath::Max(SpawnHeight, 0.f);
		const float ClearanceRadius = FMath::Max(SpawnClearanceRadius, 0.f);
		if (ClearanceRadius > UE_KINDA_SMALL_NUMBER
			&& GetWorld()->OverlapAnyTestByObjectType(
			SpawnLocation,
			FQuat::Identity,
			HitboxQueryParams,
			FCollisionShape::MakeSphere(ClearanceRadius),
			QueryParams))
		{
			continue;
		}

		FVector FacingDirection = AreaCenter - SpawnLocation;
		FacingDirection.Z = 0.f;
		FRotator SpawnRotation = FacingDirection.IsNearlyZero()
			? GetActorRotation()
			: FacingDirection.Rotation();
		SpawnRotation.Pitch = 0.f;
		SpawnRotation.Roll = 0.f;
		OutArrivalSpawn.SpawnTransform = FTransform(SpawnRotation, SpawnLocation);
		OutArrivalSpawn.GroundLocation = GroundHit.ImpactPoint;
		return true;
	}

	ensureMsgf(false,
		TEXT("Lobby Hub Arrival Volume could not find an unoccupied WorldStatic floor portal."));
	return false;
}
