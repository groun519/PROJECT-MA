#pragma once

#include "CoreMinimal.h"
#include "AlphaBlend.h"
#include "Player/MAPlayerCharacter.h"
#include "LobbyHubCharacter.generated.h"

UENUM()
enum class ELobbyHubArrivalPhase : uint8
{
	Inactive,
	Ragdoll,
	Recovering
};

/** Authoritative Hub arrival state and the parameters required to enter its current phase. */
USTRUCT()
struct FLobbyHubArrivalState
{
	GENERATED_BODY()

	UPROPERTY()
	ELobbyHubArrivalPhase Phase = ELobbyHubArrivalPhase::Inactive;

	UPROPERTY()
	FVector InitialVelocity = FVector::ZeroVector;

	UPROPERTY()
	FVector RecoveryLocation = FVector::ZeroVector;

	UPROPERTY()
	FRotator RecoveryRotation = FRotator::ZeroRotator;
};

UCLASS()
class P_MA_API ALobbyHubCharacter : public AMAPlayerCharacter
{
	GENERATED_BODY()

public:
	ALobbyHubCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Initializes only the runtime state used by the Hub. Combat initialization is intentionally excluded. */
	void InitializeHubRuntime();
	/** Begins and owns this Character's complete Hub-only Arrival lifecycle. */
	bool BeginArrival(const FVector& InitialVelocity, const FVector& GroundLocation);

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Hub|Movement", meta = (ClampMin = "0.0"))
	float HubMoveSpeed = 375.f;

	/** Arrival Lifecycle **/
	void BeginRecovery();
	void FinishArrival();
	void SetArrivalPhase(ELobbyHubArrivalPhase NewPhase);
	void ApplyArrivalPhase();

	UFUNCTION()
	void OnRep_ArrivalState();

	/** Replicated Arrival Phases **/
	void StartRagdoll();
	void StartRecovery();
	void RestoreHubControl();

	/** Ragdoll **/
	void UpdateRagdoll(float DeltaSeconds);

	/** Recovery **/
	FTransform ResolveRecoveryTransform() const;
	void BeginRecoveryMeshBlend();
	void UpdateRecoveryMeshBlend(float DeltaSeconds);
	void UpdateArrivalNetworkSmoothing(bool bIsRecovering);

	/** Arrival Configuration **/
	/** Blends the preserved ragdoll Mesh alignment back to its authored relative transform. */
	UPROPERTY(EditDefaultsOnly, Category = "Hub|Arrival", meta = (ClampMin = "0.0", Units = "s"))
	float ArrivalMeshBlendDuration = 0.5f;

	/** Pelvis speed below which this Character considers its Ragdoll stable. */
	UPROPERTY(EditDefaultsOnly, Category = "Hub|Arrival", meta = (ClampMin = "0.0", Units = "cm/s"))
	float ArrivalSettleSpeed = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "Hub|Arrival", meta = (ClampMin = "0.0", Units = "s"))
	float ArrivalSettleDuration = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Hub|Arrival", meta = (ClampMin = "0.1", Units = "s"))
	float MaxRagdollArrivalTime = 4.f;

	/** Authored Character State **/
	FTransform DefaultMeshRelativeTransform = FTransform::Identity;
	FName DefaultMeshCollisionProfileName = NAME_None;

	/** Replicated Arrival State **/
	UPROPERTY(ReplicatedUsing = OnRep_ArrivalState)
	FLobbyHubArrivalState ArrivalState;

	/** Ragdoll Runtime **/
	FVector ArrivalGroundLocation = FVector::ZeroVector;
	float ArrivalMinimumSettleTime = 0.f;
	float ArrivalElapsedTime = 0.f;
	float ArrivalStableTime = 0.f;

	/** Recovery Runtime **/
	FTransform RecoveryMeshStartRelativeTransform = FTransform::Identity;
	FAlphaBlend RecoveryMeshBlend;
	bool bRecoveryMeshBlendActive = false;
	bool bHasSavedArrivalNetworkSmoothingMode = false;
	ENetworkSmoothingMode SavedArrivalNetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
};
