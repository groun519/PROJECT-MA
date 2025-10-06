// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/GAM_Jump.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameFramework/Character.h"
#include "GAS/MAAbilitySystemStatics.h"


UGAM_Jump::UGAM_Jump()
{
	RotationLock = UMAAbilitySystemStatics::GetRotationLockTag();
}

void UGAM_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay);
	if (MontageTask)
	{
		MontageTask->OnCompleted.AddDynamic(this, &UGAM_Jump::K2_EndAbility);
		MontageTask->OnBlendOut.AddDynamic(this, &UGAM_Jump::K2_EndAbility);
		MontageTask->OnInterrupted.AddDynamic(this, &UGAM_Jump::K2_EndAbility);
		MontageTask->OnCancelled.AddDynamic(this, &UGAM_Jump::K2_EndAbility);
		MontageTask->ReadyForActivation();
	}

	EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.Start"));
	if (EventTask)
	{
		EventTask->EventReceived.AddDynamic(this, &UGAM_Jump::OnStartTagReceived);
		EventTask->ReadyForActivation();
	}
	UAbilityTask_WaitGameplayEvent* SlamTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, FGameplayTag::RequestGameplayTag("Ability.Movement.Jump.End"));
	if (SlamTask)
	{
		SlamTask->EventReceived.AddDynamic(this, &UGAM_Jump::OnEndTagReceived);
		SlamTask->ReadyForActivation();
	}
}

void UGAM_Jump::OnStartTagReceived(FGameplayEventData Data)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->HasAuthority())
	{
		// 마우스 위치 대신, 캐릭터가 현재 바라보는 정면 방향을 가져옵니다.
		const FVector ForwardDirection = Character->GetActorForwardVector();
		
		// 이 방향으로 전방 및 수직 힘을 조합해 캐릭터를 발사합니다.
		FVector LaunchVelocity = ForwardDirection * ForwardLaunchForce;
		LaunchVelocity.Z = VerticalLaunchForce;

		Character->LaunchCharacter(LaunchVelocity, true, true);
	}
}

void UGAM_Jump::OnEndTagReceived(FGameplayEventData Data)
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->HasAuthority())
	{
		// 1. 위로 살짝 띄웁니다.
		const FVector HopVelocity(0.f, 0.f, SlamHopForce);
		Character->LaunchCharacter(HopVelocity, false, true);
	}

	// 2. SlamDelay 만큼 기다리는 딜레이 태스크를 생성합니다.
	UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, SlamDelay);
	if (DelayTask)
	{
		// 3. 딜레이가 끝나면 ExecuteSlam 함수를 호출하도록 예약합니다.
		DelayTask->OnFinish.AddDynamic(this, &UGAM_Jump::ExecuteSlam);
		DelayTask->ReadyForActivation();
	}
	else
	{
		// 딜레이 태스크 생성에 실패하면 그냥 어빌리티를 종료합니다.
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UGAM_Jump::ExecuteSlam()
{
	ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (Character && Character->HasAuthority())
	{
		// 4. 아래 방향으로 강력한 힘을 줍니다.
		const FVector SlamVelocity(0.f, 0.f, -SlamForce);
		Character->LaunchCharacter(SlamVelocity, false, true);
	}
}

void UGAM_Jump::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                           const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
