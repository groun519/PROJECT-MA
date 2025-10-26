// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/MovementBehavior_Blink.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GameFramework/PlayerController.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "GAS/Movement/MATargetActor_Movement.h"
#include "Player/MAPlayerCharacter.h"

void UMovementBehavior_Blink::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();

	bHasValidTargetLocation = false;
	bBlinkTagReceived = false;
	
	WaitBlinkTagEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, BlinkTag);
	WaitBlinkTagEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Blink::OnBlinkTagReceived);
	WaitBlinkTagEventTask->ReadyForActivation();

	WaitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(OwningAbility,NAME_None,EGameplayTargetingConfirmation::Instant,TargetActorClass);
	WaitTargetDataTask->ValidData.AddDynamic(this, &UMovementBehavior_Blink::TargetConfirmed);
	WaitTargetDataTask->Cancelled.AddDynamic(this, &UMovementBehavior_Blink::TargetCancelled);
	WaitTargetDataTask->ReadyForActivation();
	
	AGameplayAbilityTargetActor* SpawnedTargetActor;
	WaitTargetDataTask->BeginSpawningActor(OwningAbility, TargetActorClass, SpawnedTargetActor);
	WaitTargetDataTask->FinishSpawningActor(OwningAbility, SpawnedTargetActor);

	WaitDamageTagEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
	WaitDamageTagEventTask->EventReceived.AddDynamic(this, &UMovementBehavior_Blink::OnDamageEventReceived);
	WaitDamageTagEventTask->ReadyForActivation();
}

void UMovementBehavior_Blink::OnEndAbility_Implementation()
{
	if (WaitBlinkTagEventTask.IsValid())
		WaitBlinkTagEventTask->EndTask();
	if (WaitDamageTagEventTask.IsValid())
		WaitDamageTagEventTask->EndTask();
	if (WaitTargetDataTask.IsValid())
		WaitTargetDataTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}

void UMovementBehavior_Blink::OnBlinkTagReceived(FGameplayEventData Payload)
{
	bBlinkTagReceived = true;
	TryTeleport();
}


void UMovementBehavior_Blink::TargetConfirmed(const FGameplayAbilityTargetDataHandle& Data)
{
	if (!Character)
		return;
	
	FHitResult HitResult = *Data.Get(0)->GetHitResult();
	FVector StartLocation = Character->GetActorLocation();
	FVector HitLocation = HitResult.ImpactPoint;

	FVector Direction = HitLocation - StartLocation;
	Direction.Z = 0;
	float Distance = FMath::Min(Direction.Size(), MaxBlinkDistance);
	Direction.Normalize();

	// [1. 텔레포트하지 않고, 계산된 위치를 변수에 "저장"만 합니다.]
	CachedBlinkLocation = StartLocation + (Direction * Distance);
	CachedBlinkRotation = Direction.Rotation();
	
	// 2. "위치" 준비 완료
	bHasValidTargetLocation = true; 

	// 3. "위치"와 "타이밍"이 모두 준비되었는지 확인
	TryTeleport();
}

void UMovementBehavior_Blink::TargetCancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	OwningAbility->RequestEndAbility();
}


void UMovementBehavior_Blink::OnDamageEventReceived(FGameplayEventData EventData)
{
	TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(EventData.TargetData);
	for (FHitResult& HitResult : HitResults)
	{
		OwningAbility->ApplyGameplayEffectToHitResultActor(HitResult, MovementDamageEffect, OwningAbility->GetAbilityLevel());
	}
}

void UMovementBehavior_Blink::TryTeleport()
{
	if (bHasValidTargetLocation && bBlinkTagReceived)
	{
		if (Character)
		{
			Character->TeleportTo(CachedBlinkLocation, CachedBlinkRotation);
		}
		
		bHasValidTargetLocation = false;
		bBlinkTagReceived = false;
	}
}

