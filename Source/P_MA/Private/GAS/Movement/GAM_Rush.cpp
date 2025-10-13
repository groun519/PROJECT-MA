// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Movement/GAM_Rush.h"
#include "AbilitySystemComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "TimerManager.h" // 타이머 사용을 위해 헤더 추가

UGAM_Rush::UGAM_Rush()
{
	
}

void UGAM_Rush::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    // Super::ActivateAbility를 먼저 호출하는 것이 좋습니다.
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!K2_CommitAbility())
	{
		K2_EndAbility();
		return;
	}

	AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (Character)
	{
		GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(Character->RushingTag);
		bIsEnd=false;
		
		// 추가된 부분 
		// 1. UI에게 차지 신호 보냄
		Character->OnChargeAbilityStarted.Broadcast();

		// 2. 타이머 시작
		StartTime = GetWorld()->GetTimeSeconds();
		GetWorld()->GetTimerManager().SetTimer(ChargeUpdateTimerHandle, this, &UGAM_Rush::UpdateChargeUI, 0.02f, true);
		// 여기까지
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None,MontageToPlay);
	if (MontageTask)
	{
		MontageTask->OnBlendOut.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->OnCancelled.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->OnCompleted.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->OnInterrupted.AddDynamic(this, &UGAM_Rush::K2_EndAbility);
		MontageTask->ReadyForActivation();
	}

	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	if (WaitInputReleaseTask)
	{
		WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGAM_Rush::OnInputReleased);
		WaitInputReleaseTask->ReadyForActivation();
	}

	TimeoutTask = UAbilityTask_WaitDelay::WaitDelay(this,MaxHoldDuration);
	if (TimeoutTask)
	{
		TimeoutTask -> OnFinish.AddDynamic(this, &UGAM_Rush::OnTimeout);
		TimeoutTask -> ReadyForActivation();
	}
	
	AttackEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this,FGameplayTag::RequestGameplayTag("Ability.Movement.Damage"));
	if (AttackEventTask)
	{
		AttackEventTask->EventReceived.AddDynamic(this, &UGAM_Rush::DoDamage);
		AttackEventTask->ReadyForActivation();
	}
}

// 추가된 부분 
void UGAM_Rush::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 타이머 정리
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateTimerHandle);

	AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (Character)
	{
		// UI에게 충전 끝 신호
		Character->OnChargeAbilityEnded.Broadcast();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGAM_Rush::UpdateChargeUI()
{
	AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
    if (Character)
    {
        // 경과 시간을 기반으로 충전 진행률(0.0 ~ 1.0) 계산
        const float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
        const float ChargePercentage = FMath::Clamp(ElapsedTime / MaxHoldDuration, 0.0f, 1.0f);
        
        // UI에게 현재 진행률 방송
        Character->OnChargeAbilityUpdate.Broadcast(ChargePercentage);
    }
}
// 여기까지 추가

void UGAM_Rush::OnInputPressed(float TimePressed)
{
	WaitInputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	WaitInputReleaseTask->OnRelease.AddDynamic(this, &UGAM_Rush::OnInputReleased);
	WaitInputReleaseTask->ReadyForActivation();
}

void UGAM_Rush::OnInputReleased(float TimePressed)
{
	if (bIsEnd)
		return;
	bIsEnd=true;
	
	MontageToEndSection();
}

void UGAM_Rush::OnTimeout()
{
	if (bIsEnd) // 중복 호출 방지
		return;
	bIsEnd=true;
	MontageToEndSection();
}

void UGAM_Rush::MontageToEndSection()
{
	AMAPlayerCharacter* Character = Cast<AMAPlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (Character)
	{
		GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(Character->RushingTag);
		if (MontageToPlay)
		{
			Character->GetMesh()->GetAnimInstance()->Montage_JumpToSection("End",MontageToPlay);
		}
	}
}