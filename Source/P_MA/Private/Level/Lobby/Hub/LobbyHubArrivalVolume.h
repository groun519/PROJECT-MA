#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyHubArrivalVolume.generated.h"

class USphereComponent;
class ALobbyHubCharacter;

/** Valid floor portal and Pawn spawn resolved for one Hub arrival. */
struct FLobbyHubArrivalSpawn
{
	FTransform SpawnTransform = FTransform::Identity;
	FVector GroundLocation = FVector::ZeroVector;
};

/** Owns the circular Hub arrival area, free portal selection, and initial launch. */
UCLASS()
class P_MA_API ALobbyHubArrivalVolume : public AActor
{
	GENERATED_BODY()

public:
	ALobbyHubArrivalVolume();

	/** Resolves one unoccupied floor portal and its Pawn spawn transform. */
	bool TryCreateArrivalSpawn(FLobbyHubArrivalSpawn& OutArrivalSpawn) const;
	/** Chooses the initial launch speed and hands the complete lifecycle to the Character. */
	void Launch(ALobbyHubCharacter& Character, const FLobbyHubArrivalSpawn& ArrivalSpawn) const;

private:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USphereComponent> ArrivalArea;

	/** Minimum initial upward speed applied when the ragdoll leaves its floor portal. */
	UPROPERTY(EditAnywhere, Category = "Arrival", meta = (ClampMin = "0.0", Units = "cm/s"))
	float MinLaunchSpeed = 500.f;

	/** Maximum initial upward speed applied when the ragdoll leaves its floor portal. */
	UPROPERTY(EditAnywhere, Category = "Arrival", meta = (ClampMin = "0.0", Units = "cm/s"))
	float MaxLaunchSpeed = 1000.f;

	/** Keeps the initial Pawn capsule above the selected portal floor. */
	UPROPERTY(EditAnywhere, Category = "Arrival", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnHeight = 100.f;

	/** Rejects portal candidates occupied by a Hitbox within this radius. */
	UPROPERTY(EditAnywhere, Category = "Arrival", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnClearanceRadius = 75.f;
};
