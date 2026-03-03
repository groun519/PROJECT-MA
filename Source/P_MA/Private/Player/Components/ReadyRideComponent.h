// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReadyRideComponent.generated.h"

class AMAPlayerCharacter;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UReadyRideComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UReadyRideComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void NotifyReadyRideAttachmentChanged(bool bInAttachedReady);
	bool IsAttachedReady() const;
	bool ShouldBlockManualRotation() const { return IsAttachedReady(); }
	bool TryGetAttachedYaw(float& OutYaw) const;
	float GetAttachedMoveSpeed() const { return AttachedMoveSpeed; }
	FVector GetAttachedMoveVelocity() const { return AttachedMoveVelocity; }

	void RefreshRideCollisionMode();
	void HandleOwnerBaseChanged();

private:
	bool IsAttachedToPlatformRoot() const;
	void UpdateRideCollisionWithOtherPlayer(AMAPlayerCharacter* OwnerCharacter, AMAPlayerCharacter* OtherPlayer) const;
	void HandleReplicatedAttachStateChanged(bool bNowAttached) const;

	bool bIsRidingPlatform = false;
	bool bPrevAttachedReady = false;
	float AttachedMoveSpeed = 0.f;
	FVector AttachedMoveVelocity = FVector::ZeroVector;
	FVector PrevTickLocation = FVector::ZeroVector;

	// TODO(Mount): Extend ride-base detection/collision policy for mount actors.
};
