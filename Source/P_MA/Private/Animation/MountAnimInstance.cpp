// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/MountAnimInstance.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/Components/ReadyRideComponent.h"

void UMountAnimInstance::NativeInitializeAnimation()
{
	OwnerPlayerCharacter = Cast<AMAPlayerCharacter>(GetOwningActor());
}

void UMountAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerPlayerCharacter)
	{
		OwnerPlayerCharacter = Cast<AMAPlayerCharacter>(GetOwningActor());
	}

	if (!OwnerPlayerCharacter)
	{
		Speed = 0.f;
		RideHorizontalInput = 0.f;
		return;
	}

	const UReadyRideComponent* ReadyRideComponent = OwnerPlayerCharacter->GetReadyRideComponent();
	Speed = ReadyRideComponent ? ReadyRideComponent->GetRideMoveSpeed() : 0.f;
	RideHorizontalInput = OwnerPlayerCharacter->GetRideHorizontalInput();
}
