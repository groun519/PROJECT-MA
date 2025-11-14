// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_Hold.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Player/MAPlayerCharacter.h"

void USkillBehavior_Hold::OnActivate_Implementation()
{
	if (!OwningAbility)
		return;
	Super::OnActivate_Implementation();
	
	if (PlayerCharacter)
	{
		PlayerCharacter->OnChargeAbilityStarted.Broadcast();
		StartTime = GetWorld()->GetTimeSeconds();
		GetWorld()->GetTimerManager().SetTimer(ChargeUpdateTimerHandle, this, &USkillBehavior_Hold::UpdateChargeUI,0.02f, true);
	}
	bIsHoldEnd = false;

	//최대 홀딩 시간
	HoldTimeOut = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, MaxHoldDuration);
	HoldTimeOut->OnFinish.AddDynamic(this, &USkillBehavior_Hold::OnMaxHold);
	HoldTimeOut->ReadyForActivation();
	//홀딩 중 키 놓으면
	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	InputReleaseTask->OnRelease.AddDynamic(this, &USkillBehavior_Hold::OnHoldReleased);
	InputReleaseTask->ReadyForActivation();
	//Ignore Target 배열 초기화
	WaitClearEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, IgnoreClearTag);
	WaitClearEventTask->EventReceived.AddDynamic(this, &USkillBehavior_Hold::ClearIgnore);
	WaitClearEventTask->ReadyForActivation();
	if (OwningAbility->K2_HasAuthority())
	{
		//데미지 태그 만나면
		WaitHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
		WaitHitEventTask->EventReceived.AddDynamic(this, &USkillBehavior_Hold::HitTarget);
		WaitHitEventTask->ReadyForActivation();
	}
}

void USkillBehavior_Hold::OnEndAbility_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateTimerHandle);
	if (PlayerCharacter)
		PlayerCharacter->OnChargeAbilityEnded.Broadcast();
	
	if (HoldTimeOut.IsValid())
		HoldTimeOut->EndTask();
	if (InputReleaseTask.IsValid())
		InputReleaseTask->EndTask();
	if (WaitHitEventTask.IsValid())
		WaitHitEventTask->EndTask();
	if (WaitClearEventTask.IsValid())
		WaitClearEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}


void USkillBehavior_Hold::OnHoldReleased(float Time)
{
	if (bIsHoldEnd)
		return;
	bIsHoldEnd = true;

	if (Time <= 0.2f)
	{
		OwningAbility->ApplyShortCooldownAndRequestEndAbility();
		return;
	}
	OwningAbility->ApplyDefaultCooldownOnce();
	MontageToOtherSection(FName("End"));
}

void USkillBehavior_Hold::OnMaxHold()
{
	if (bIsHoldEnd)
		return;
	bIsHoldEnd = true;
	OwningAbility->ApplyDefaultCooldownOnce();
	MontageToOtherSection(FName("End"));
}

void USkillBehavior_Hold::HitTarget(FGameplayEventData EventData)
{
	if (OwningAbility->K2_HasAuthority())
	{
		TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(EventData.TargetData);
		OwningAbility->ApplyDamageToHitResults(HitResults, DamageEffect);
	}
}

void USkillBehavior_Hold::ClearIgnore(FGameplayEventData EventData)
{
	if (OwningAbility->K2_HasAuthority())
	{
		OwningAbility->IgnoreTargets.Empty();
	}
}

void USkillBehavior_Hold::UpdateChargeUI()
{
	if (PlayerCharacter)
	{
		// 경과 시간을 기반으로 충전 진행률(0.0 ~ 1.0) 계산
		const float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
		const float ChargePercentage = FMath::Clamp(ElapsedTime / MaxHoldDuration, 0.0f, 1.0f);
        
		// UI에게 현재 진행률 방송
		PlayerCharacter->OnChargeAbilityUpdate.Broadcast(ChargePercentage);
	}
}
