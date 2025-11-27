// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/Movement/MovementBehavior_Jump.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Character/MACharacter.h"
#include "Components/CapsuleComponent.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GAS/Movement/MATargetActor_Movement.h"

void UMovementBehavior_Jump::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	bJumpTagReceived = false;
	bHasValidTargetLocation = false;

	// 점프 실행 태그 대기
	WaitJumpStartEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, JumpStartTag);
	WaitJumpStartEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Jump::OnJumpStartEventReceived);
	WaitJumpStartEventTask->ReadyForActivation();

	// 점프 종료 태그 대기
	WaitJumpEndEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, JumpEndTag);
	WaitJumpEndEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Jump::OnJumpEndEventReceived);
	WaitJumpEndEventTask->ReadyForActivation();

	// 클라이언트에게 마우스 위치 즉시 요청(TargetActor_Movement)
	WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(OwningAbility, NAME_None, EGameplayTargetingConfirmation::Instant, TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this, &UMovementBehavior_Jump::TargetConfirmed);
	WaitTargetDataTask->ReadyForActivation();

	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetDataTask->BeginSpawningActor(OwningAbility, TargetActorClass, TargetActor);
	WaitTargetDataTask->FinishSpawningActor(OwningAbility, TargetActor);

	if (OwningAbility->K2_HasAuthority())
	{
		// 데미지 태그 대기
		WaitDamageTagEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
		WaitDamageTagEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Jump::OnDamageEventReceived);
		WaitDamageTagEventTask->ReadyForActivation();
	}
}

void UMovementBehavior_Jump::OnEndAbility_Implementation()
{
	Character->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	
	if (WaitTargetDataTask.IsValid())
		WaitTargetDataTask->EndTask();
	if (WaitJumpStartEventTask.IsValid())
		WaitJumpStartEventTask->EndTask();
	if (WaitJumpEndEventTask.IsValid())
		WaitJumpEndEventTask->EndTask();
	if (WaitDamageTagEventTask.IsValid())
		WaitDamageTagEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void UMovementBehavior_Jump::InitFromData(const FSkillDefinitionDT& Data)
{
	Super::InitFromData(Data);

	MontageToPlay = Data.JumpData.MontageToPlay;
	TargetActorClass=Data.JumpData.TargetActorClass;
	
	if (Data.JumpData.DamageMultiplier>0.f)		BehaviorDamageMultiplier = Data.JumpData.DamageMultiplier;
	if (Data.JumpData.CooldownDuration>0.f)		CooldownDuration = Data.JumpData.CooldownDuration;
	
	if (Data.JumpData.MaxJumpDistance>0.f)		MaxJumpDistance = Data.JumpData.MaxJumpDistance;
	if (Data.JumpData.MinJumpDistance>0.f)		MinJumpDistance = Data.JumpData.MinJumpDistance;
	if (Data.JumpData.MaxJumpForce>0.f)			MaxJumpForce = Data.JumpData.MaxJumpForce;
	if (Data.JumpData.MinJumpForce>0.f)			MinJumpForce = Data.JumpData.MinJumpForce;
	if (Data.JumpData.VerticalLaunchForce>0.f)	VerticalLaunchForce = Data.JumpData.VerticalLaunchForce;
	if (Data.JumpData.SlamForce>0.f)			SlamForce = Data.JumpData.SlamForce;
}

void UMovementBehavior_Jump::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	if (!Character)
		return;

	FHitResult HitResult = *Data.Get(0)->GetHitResult();
	FVector StartLocation = Character->GetActorLocation();
	FVector HitLocation = HitResult.ImpactPoint;

	FVector Direction = HitLocation - StartLocation;
	Direction.Z = 0;
	float Distance = FMath::Min(Direction.Size(), MaxJumpDistance);
	Direction.Normalize();

	CachedJumpLocation = StartLocation + (Direction * Distance);
	bHasValidTargetLocation = true;

}

void UMovementBehavior_Jump::OnJumpStartEventReceived(FGameplayEventData EventData)
{
	bJumpTagReceived = true;
	TryJump();
}

void UMovementBehavior_Jump::OnJumpEndEventReceived(FGameplayEventData EventData)
{
	if (Character && Character->HasAuthority())
	{
		const FVector SlamVel(0.f, 0.f, SlamForce);
		Character->LaunchCharacter(SlamVel, false, true);
	}
}

void UMovementBehavior_Jump::OnDamageEventReceived(FGameplayEventData EventData)
{
	TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(EventData.TargetData);
	OwningAbility->ApplyDamageToHitResults(HitResults);

}

void UMovementBehavior_Jump::TryJump()
{
	if (bHasValidTargetLocation && bJumpTagReceived)
	{
		if (!Character)
			return;

		const FVector StartLocation = Character->GetActorLocation();
		const FVector TargetLocation = CachedJumpLocation;

		FVector Direction = TargetLocation - StartLocation;
		Direction.Z = 0;
		const float Distance = Direction.Size();
		Direction.Normalize();

		const float CalculatedForce = FMath::GetMappedRangeValueClamped(
			FVector2D(MinJumpDistance, MaxJumpDistance),
			FVector2D(MinJumpForce, MaxJumpForce),
			Distance);

		FVector LaunchVel = Direction * CalculatedForce;
		LaunchVel.Z = VerticalLaunchForce;
		
		Character->LaunchCharacter(LaunchVel, true,true);

		bHasValidTargetLocation = false;
		bJumpTagReceived = false;
	}
}