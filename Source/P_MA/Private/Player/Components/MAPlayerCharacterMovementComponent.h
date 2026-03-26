// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MAPlayerCharacterMovementComponent.generated.h"

UENUM()
enum class ERideCustomMovementMode : uint8
{
	None,
	Ride
};

UCLASS()
class P_MA_API UMAPlayerCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	void SetRideMovementEnabled(bool bEnabled);
	bool IsRideMovementActive() const;

	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

private:
	void PhysRide(float DeltaTime, int32 Iterations);
};
