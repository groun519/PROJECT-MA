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

	void NotifyReadyRideAttachmentChanged(ARideRoot* InRideRoot);
	bool IsAttachedReady() const;
	bool ShouldBlockManualRotation() const { return IsAttachedReady(); }
	bool TryGetAttachedYaw(float& OutYaw) const;
	float GetAttachedMoveSpeed() const { return AttachedMoveSpeed; }
	FVector GetAttachedMoveVelocity() const { return AttachedMoveVelocity; }
	ERideMountState GetMountState() const { return MountState; }

	void RefreshRideCollisionMode();
	void HandleOwnerBaseChanged();

private:
	void UpdateRideCollisionWithOtherPlayer(AMAPlayerCharacter* OwnerCharacter, AMAPlayerCharacter* OtherPlayer) const;
	void HandleReplicatedAttachStateChanged(bool bNowAttached);
	void ApplyRideState(bool bNowAttached);
	void UpdateRideMovementMode(bool bNowAttached) const;
	void UpdateTickPolicy(bool bAttachedByReady);
	void UpdateMountState(bool bNowAttached);
	void UpdateProxyMeshSmoothing(bool bNowAttached);
	void CacheOwnerMeshAttachment();
	void AttachOwnerMeshToMount();
	void RestoreOwnerMeshAttachment();

	UFUNCTION()
	void OnRep_RidingRoot();

	UPROPERTY(ReplicatedUsing=OnRep_RidingRoot)
	TObjectPtr<ARideRoot> RidingRoot = nullptr;

	bool bIsRidingPlatform = false;
	bool bPrevAttachedReady = false;
	float AttachedMoveSpeed = 0.f;
	FVector AttachedMoveVelocity = FVector::ZeroVector;
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
