// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/MovementBehavior_Jump.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "Player/MAPlayerCharacter.h"

void UMovementBehavior_Jump::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	WaitJumpStartEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, JumpStartTag);
	WaitJumpStartEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Jump::OnJumpStartEventReceived);
	WaitJumpStartEventTask->ReadyForActivation();

	WaitJumpEndEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, JumpStartTag);
	WaitJumpEndEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Jump::OnJumpEndEventReceived);
	WaitJumpEndEventTask->ReadyForActivation();

	WaitDamageTagEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
	WaitDamageTagEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Jump::OnDamageEventReceived);
	WaitDamageTagEventTask->ReadyForActivation();
}

void UMovementBehavior_Jump::OnEndAbility_Implementation()
{
	if (WaitJumpStartEventTask.IsValid())
		WaitJumpStartEventTask->EndTask();
	if (WaitJumpEndEventTask.IsValid())
		WaitJumpEndEventTask->EndTask();
	if (WaitDamageTagEventTask.IsValid())
		WaitDamageTagEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void UMovementBehavior_Jump::OnJumpStartEventReceived(FGameplayEventData EventData)
{
	if (!PlayerCharacter || !PlayerCharacter->IsLocallyControlled())
		return;

	APlayerController* PC = Cast<APlayerController>(PlayerCharacter->GetController());
	if (PC)
	{
		FHitResult HitResult;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, true, HitResult))
		{
			const FVector TargetLocation = HitResult.ImpactPoint;
			const FVector StartLocation = PlayerCharacter->GetActorLocation();

			FVector Direction = TargetLocation - StartLocation;
			Direction.Z =0;
			const float Distance = Direction.Size();
			Direction.Normalize();

			const float CalculatedForce = FMath::GetMappedRangeValueClamped(
				FVector2D(0.f, MaxJumpDistance),
				FVector2D(MinJumpForce, MaxJumpForce),
				Distance);

			FVector LaunchVel = Direction * CalculatedForce;
			LaunchVel.Z = VerticalLaunchForce;

			PlayerCharacter->Server_RequestLaunch(LaunchVel, true,true);
		}
	}
}

void UMovementBehavior_Jump::OnJumpEndEventReceived(FGameplayEventData EventData)
{
}

void UMovementBehavior_Jump::OnDamageEventReceived(FGameplayEventData EventData)
{
}

void UMovementBehavior_Jump::ExecuteSlam()
{
}

