#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MAProjectileMovementComponent.generated.h"

UCLASS()
class P_MA_API UMAProjectileMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

protected:
	virtual FVector ComputeHomingAcceleration(const FVector& InVelocity, float DeltaTime) const override;
};
