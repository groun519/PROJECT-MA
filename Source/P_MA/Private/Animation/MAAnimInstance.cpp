// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/MAAnimInstance.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/Components/ReadyRideComponent.h"

void UMAAnimInstance::NativeInitializeAnimation()
{
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComp = OwnerCharacter->GetCharacterMovement();
	}
}

void UMAAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (OwnerCharacter)
	{
		if (const AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(OwnerCharacter))
		{
			if (const UReadyRideComponent* ReadyRideComp = PlayerCharacter->GetReadyRideComponent();
				ReadyRideComp && ReadyRideComp->ShouldBlockManualRotation())
			{
				Velocity = ReadyRideComp->GetAttachedMoveVelocity();
				Speed = ReadyRideComp->GetAttachedMoveSpeed();
			}
			else
			{
				Velocity = OwnerCharacter->GetVelocity();
				Speed = Velocity.Length();
			}
		}
		else
		{
			Velocity = OwnerCharacter->GetVelocity();
			Speed = Velocity.Length();
		}
		FRotator BodyRot = OwnerCharacter->GetActorRotation();
		BodyPrevRot = BodyRot;

		FRotator ControlRot = OwnerCharacter->GetBaseAimRotation();
		LookRotOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, BodyRot);
	}
}

void UMAAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{

}
