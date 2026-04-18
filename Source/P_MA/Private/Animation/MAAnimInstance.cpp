// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/MAAnimInstance.h"
#include "GAS/Skill/MASkillAbility.h"
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
				ReadyRideComp)
			{
				bIsMounted = ReadyRideComp->GetMountState() == ERideMountState::Mounted;
				const bool bUseRideMovementSource = ReadyRideComp->IsRiding();
				if (bUseRideMovementSource)
				{
					Velocity = ReadyRideComp->GetRideMoveVelocity();
					Speed = ReadyRideComp->GetRideMoveSpeed();
				}
				else
				{
					Velocity = OwnerCharacter->GetVelocity();
					Speed = Velocity.Length();
				}
			}
			else
			{
				bIsMounted = false;
				Velocity = OwnerCharacter->GetVelocity();
				Speed = Velocity.Length();
			}
		}
		else
		{
			bIsMounted = false;
			Velocity = OwnerCharacter->GetVelocity();
			Speed = Velocity.Length();
		}
		FRotator BodyRot = OwnerCharacter->GetActorRotation();
		BodyPrevRot = BodyRot;

		FRotator ControlRot = OwnerCharacter->GetBaseAimRotation();
		LookRotOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, BodyRot);
	}
	else
	{
		bIsMounted = false;
	}
}

void UMAAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{

}

void UMAAnimInstance::RegisterAnimationOwner(const UAnimSequenceBase* Animation, UMASkillAbility* SkillAbility)
{
	if (!Animation || !SkillAbility) return;
	AnimationOwners.FindOrAdd(Animation) = SkillAbility;
}

UMASkillAbility* UMAAnimInstance::FindAnimationOwner(const UAnimSequenceBase* Animation) const
{
	if (!Animation) return nullptr;

	const TWeakObjectPtr<UMASkillAbility>* FoundOwner = AnimationOwners.Find(Animation);
	return FoundOwner ? FoundOwner->Get() : nullptr;
}

void UMAAnimInstance::UnregisterAnimationOwner(const UAnimSequenceBase* Animation, const UMASkillAbility* SkillAbility)
{
	if (!Animation) return;

	const TWeakObjectPtr<UMASkillAbility>* FoundOwner = AnimationOwners.Find(Animation);
	if (!FoundOwner) return;
	if (SkillAbility && FoundOwner->Get() != SkillAbility) return;

	AnimationOwners.Remove(Animation);
}
