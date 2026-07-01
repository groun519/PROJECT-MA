#include "GAS/Projectile/MAProjectileMovementComponent.h"

FVector UMAProjectileMovementComponent::ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const
{
	const FVector TargetDirection = (
		HomingTargetComponent->GetComponentLocation() - UpdatedComponent->GetComponentLocation()).GetSafeNormal();
	const FVector DesiredVelocity = TargetDirection * GetMaxSpeed();
	const FVector RequiredAcceleration = (DesiredVelocity - InVelocity) / FMath::Max(DeltaTime, UE_SMALL_NUMBER);

	return RequiredAcceleration.GetClampedToMaxSize(HomingAccelerationMagnitude);
}
