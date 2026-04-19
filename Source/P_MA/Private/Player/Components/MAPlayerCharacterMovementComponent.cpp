#include "Player/Components/MAPlayerCharacterMovementComponent.h"

#include "GameFramework/Character.h"

void UMAPlayerCharacterMovementComponent::SetRideMovementEnabled(bool bEnabled)
{
	const uint8 RideMode = static_cast<uint8>(ERideCustomMovementMode::Ride);
	if (bEnabled)
	{
		if (MovementMode != MOVE_Custom || CustomMovementMode != RideMode)
		{
			SetMovementMode(MOVE_Custom, RideMode);
		}
		return;
	}

	if (MovementMode == MOVE_Custom && CustomMovementMode == RideMode)
	{
		SetMovementMode(MOVE_Walking);
	}
}

bool UMAPlayerCharacterMovementComponent::IsRideMovementActive() const
{
	return MovementMode == MOVE_Custom &&
		CustomMovementMode == static_cast<uint8>(ERideCustomMovementMode::Ride);
}

float UMAPlayerCharacterMovementComponent::GetMaxSpeed() const
{
	if (IsRideMovementActive())
	{
		return MaxWalkSpeed;
	}

	return Super::GetMaxSpeed();
}

float UMAPlayerCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (IsRideMovementActive())
	{
		return BrakingDecelerationWalking;
	}

	return Super::GetMaxBrakingDeceleration();
}

void UMAPlayerCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (IsRideMovementActive())
	{
		PhysRide(DeltaTime, Iterations);
		return;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UMAPlayerCharacterMovementComponent::PhysRide(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME) return;
	if (!CharacterOwner || !UpdatedComponent) return;

	// Mounted simulated proxies should follow replicated base/root updates only.
	// Running local ride physics here double-applies movement and causes remote drift.
	if (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)
	{
		Velocity.Z = 0.f;
		return;
	}

	const bool bCanSimulateRideMovement =
		CharacterOwner->Controller ||
		bRunPhysicsWithNoController ||
		HasAnimRootMotion() ||
		CurrentRootMotion.HasOverrideVelocity();
	if (!bCanSimulateRideMovement)
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	RestorePreAdditiveRootMotionVelocity();

	if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
	{
		CalcVelocity(DeltaTime, GroundFriction, false, GetMaxBrakingDeceleration());
	}

	ApplyRootMotionToVelocity(DeltaTime);
	Velocity.Z = 0.f;

	const FVector OldLocation = UpdatedComponent->GetComponentLocation();
	const FVector Delta = Velocity * DeltaTime;

	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
	if (Hit.IsValidBlockingHit())
	{
		HandleImpact(Hit, DeltaTime, Delta);
		SlideAlongSurface(Delta, 1.f - Hit.Time, Hit.Normal, Hit, true);
	}

	Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / DeltaTime;
	Velocity.Z = 0.f;
}
