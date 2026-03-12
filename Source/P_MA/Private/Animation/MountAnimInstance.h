// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MountAnimInstance.generated.h"

UCLASS()
class P_MA_API UMountAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetSpeed() const { return Speed; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE float GetRideHorizontalInput() const { return RideHorizontalInput; }

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	FORCEINLINE bool IsMoving() const { return Speed > KINDA_SMALL_NUMBER; }

private:
	UPROPERTY()
	class AMAPlayerCharacter* OwnerPlayerCharacter = nullptr;

	float Speed = 0.f;
	float RideHorizontalInput = 0.f;
};
