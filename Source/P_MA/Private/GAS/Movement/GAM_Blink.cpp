// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/GAM_Blink.h"

#include "GameFramework/Character.h"
#include "GAS/MAAbilitySystemStatics.h"

UGAM_Blink::UGAM_Blink()
{
	RotationLock = UMAAbilitySystemStatics::GetRotationLockTag();
}

void UGAM_Blink::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGAM_Blink::K2_EndAbility);
		MontageTask->OnBlendOut.AddDynamic(this, &UGAM_Blink::K2_EndAbility);
		MontageTask->OnInterrupted.AddDynamic(this, &UGAM_Blink::K2_EndAbility);
		MontageTask->OnCancelled.AddDynamic(this, &UGAM_Blink::K2_EndAbility);
		MontageTask->ReadyForActivation();
	}

	EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Ability.Movement.Teleport.Start"));
	if (EventTask)
	{
		EventTask->EventReceived.AddDynamic(this, &UGAM_Blink::OnBlinkEventReceived);
		EventTask->ReadyForActivation();
	}
}

void UGAM_Blink::OnBlinkEventReceived(FGameplayEventData Payload)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->HasAuthority())
	{
		// 현재 위치와 바라보는 방향을 가져옵니다.
		const FVector StartLocation = Character->GetActorLocation();
		const FVector ForwardDirection = Character->GetActorForwardVector();

		// 목표 위치를 계산합니다.
		const FVector TargetLocation = StartLocation + (ForwardDirection * BlinkDistance);

		// 목표 위치로 즉시 이동합니다.
		Character->TeleportTo(TargetLocation, Character->GetActorRotation());
	}
}

void UGAM_Blink::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                            const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
