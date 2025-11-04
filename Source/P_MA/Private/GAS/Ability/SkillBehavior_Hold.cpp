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
	//애니메이션 거꾸로 재생하도록
	WaitReverseTagTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, ReversePlayTag);
	WaitReverseTagTask->EventReceived.AddDynamic(this, &USkillBehavior_Hold::OnReversePlay);
	WaitReverseTagTask->ReadyForActivation();
	//홀딩 중 키 놓으면
	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	InputReleaseTask->OnRelease.AddDynamic(this, &USkillBehavior_Hold::OnHoldReleased);
	InputReleaseTask->ReadyForActivation();

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
	if (CooldownGE)
		OwningAbility->ApplyEffectToOwner(CooldownGE);
	
	if (HoldTimeOut.IsValid())
		HoldTimeOut->EndTask();
	if (WaitForwardTagTask.IsValid())
		WaitForwardTagTask->EndTask();
	if (WaitReverseTagTask.IsValid())
		WaitReverseTagTask->EndTask();
	if (InputReleaseTask.IsValid())
		InputReleaseTask->EndTask();
	if (WaitHitEventTask.IsValid())
		WaitHitEventTask->EndTask();
	
	Super::OnEndAbility_Implementation();
}


void USkillBehavior_Hold::OnForwardPlay(FGameplayEventData EventData)
{
	if (bIsHoldEnd)
		return;
	if (OwningAbility)
		OwningAbility->SetMontagePlayRate(1.f);
	
	WaitReverseTagTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility,ReversePlayTag);
	WaitReverseTagTask->EventReceived.AddDynamic(this, &USkillBehavior_Hold::OnReversePlay);
	WaitReverseTagTask->ReadyForActivation();
}

void USkillBehavior_Hold::OnReversePlay(FGameplayEventData EventData)
{
	if (bIsHoldEnd)
		return;
	if (OwningAbility)
	OwningAbility->SetMontagePlayRate(ReverseSpeed);
	
	WaitForwardTagTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, ForwardPlayTag);
	WaitForwardTagTask->EventReceived.AddDynamic(this, &USkillBehavior_Hold::OnForwardPlay);
	WaitForwardTagTask->ReadyForActivation();
}

void USkillBehavior_Hold::OnHoldReleased(float Time)
{
	if (bIsHoldEnd)
		return;
	bIsHoldEnd = true;
	if (OwningAbility)
	{
		OwningAbility->SetMontagePlayRate(1.f);
		OwningAbility->MontageToOtherSection(FName("End"));
	}
}

void USkillBehavior_Hold::HitTarget(FGameplayEventData EventData)
{
	TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(EventData.TargetData);
	OwningAbility->ApplyDamageToHitResults(HitResults, DamageEffect);

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

void USkillBehavior_Hold::OnMaxHold()
{
	if (bIsHoldEnd)
		return;
	bIsHoldEnd = true;
	if (OwningAbility)
	{
		OwningAbility->SetMontagePlayRate(1.f);
		OwningAbility->MontageToOtherSection(FName("End"));
	}
}
