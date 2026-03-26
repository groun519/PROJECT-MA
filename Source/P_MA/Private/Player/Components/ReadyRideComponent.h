// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ReadyRideComponent.generated.h"

class AMAPlayerCharacter;
class ARideRoot;
class USceneComponent;

UENUM(BlueprintType)
enum class ERideMountState : uint8
{
	None,
	Mounted
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UReadyRideComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UReadyRideComponent();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetRidingRoot(ARideRoot* InRideRoot);
	bool IsRiding() const;
	ARideRoot* GetRidingRoot() const { return RidingRoot; }
	bool IsRideRotationLocked() const { return IsRiding(); }
	bool TryGetRideYaw(float& OutYaw) const;
	float GetRideMoveSpeed() const { return RideMoveSpeed; }
	FVector GetRideMoveVelocity() const { return RideMoveVelocity; }
	ERideMountState GetMountState() const { return MountState; }

	void RefreshRideCollisionMode();
	void HandleOwnerBaseChanged();

private:
	void SyncRideMovementBase() const;
	void UpdateRideCollisionWithOtherPlayer(AMAPlayerCharacter* OwnerCharacter, AMAPlayerCharacter* OtherPlayer) const;
	void ApplyRideState(bool bIsRiding);
	void UpdateRideMovementMode(bool bIsRiding) const;
	void UpdateTickPolicy(bool bIsRiding);
	void UpdateMountState(bool bIsRiding);
	void UpdateRemoteViewMeshSmoothing(bool bIsRiding);
	void CacheOwnerMeshAttachment();
	void AttachOwnerMeshToMount();
	void RestoreOwnerMeshAttachment();

	UFUNCTION()
	void OnRep_RidingRoot();

	UPROPERTY(ReplicatedUsing=OnRep_RidingRoot)
	TObjectPtr<ARideRoot> RidingRoot = nullptr;

	bool bIsRidingState = false;
	float RideMoveSpeed = 0.f;
	FVector RideMoveVelocity = FVector::ZeroVector;
	FVector PrevTickLocation = FVector::ZeroVector;
	ERideMountState MountState = ERideMountState::None;
	TWeakObjectPtr<USceneComponent> CachedOwnerMeshParent;
	FTransform CachedOwnerMeshRelativeTransform = FTransform::Identity;
	bool bHasCachedOwnerMeshAttachment = false;
	bool bHasSavedProxyNetworkSmoothingMode = false;
	ENetworkSmoothingMode SavedProxyNetworkSmoothingMode = ENetworkSmoothingMode::Exponential;
	static inline const FName MountSocketName = TEXT("MountSocket");

	// TODO(Mount): Extend ride-base detection/collision policy for mount actors.
};
