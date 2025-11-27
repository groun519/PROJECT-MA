// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_Charge.h"

#include "AbilitySystemComponent.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Player/MAPlayerCharacter.h"

void USkillBehavior_Charge::OnActivate_Implementation()
{
	if (!OwningAbility)
		return;
	Super::OnActivate_Implementation();

	if (PlayerCharacter)
	{
		PlayerCharacter->OnChargeAbilityStarted.Broadcast();
		StartTime = GetWorld()->GetTimeSeconds();
		GetWorld()->GetTimerManager().SetTimer(ChargeUpdateTimerHandle,this, &USkillBehavior_Charge::UpdateChargeUI,0.02f, true);
	}
	CachedChargeDuration=0.f;
	bIsEnd = false;
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
	
	//최대 차지 시간
	ChargeTimeoutTask = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, TimeoutDuration);
	ChargeTimeoutTask->OnFinish.AddDynamic(this, &USkillBehavior_Charge::OnMaxCharged);
	ChargeTimeoutTask->ReadyForActivation();
	//차징 시작
	WaitSlowTagTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, ChargeStartTag);
	WaitSlowTagTask->EventReceived.AddDynamic(this, &USkillBehavior_Charge::OnChargeEventReceived);
	WaitSlowTagTask->ReadyForActivation();
	//차지 중 키 놓으면
	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwningAbility);
	InputReleaseTask->OnRelease.AddDynamic(this, &USkillBehavior_Charge::OnChargeReleased);
	InputReleaseTask->ReadyForActivation();

	if (OwningAbility->K2_HasAuthority())
	{
		//데미지 태그 만나면
		WaitHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, DamageEventTag);
		WaitHitEventTask->EventReceived.AddDynamic(this, &USkillBehavior_Charge::HitTarget);
		WaitHitEventTask->ReadyForActivation();
	}
}

void USkillBehavior_Charge::OnEndAbility_Implementation()
{
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateTimerHandle);
	if (PlayerCharacter)
		PlayerCharacter->OnChargeAbilityEnded.Broadcast();

	CachedChargeDuration=0.f;
	if (ChargeTimeoutTask.IsValid())
		ChargeTimeoutTask->EndTask();
	if (WaitSlowTagTask.IsValid())
		WaitSlowTagTask->EndTask();
	if (InputReleaseTask.IsValid())
		InputReleaseTask->EndTask();
	if (WaitHitEventTask.IsValid())
		WaitHitEventTask->EndTask();

	Super::OnEndAbility_Implementation();
}

float USkillBehavior_Charge::GetCurrentDamageMultiplier() const
{
	return CachedChargeDuration;
}

void USkillBehavior_Charge::InitFromData(const FSkillDefinitionDT& Data)
{
	Super::InitFromData(Data);
	if (Data.ChargeData.MontageToPlay)	MontageToPlay = Data.ChargeData.MontageToPlay;
	if (Data.ChargeData.VFXDataSet)		VFXDataSet = Data.ChargeData.VFXDataSet;
	if (Data.ChargeData.MaxChargeDuration>0.f)	MaxChargeDuration = Data.ChargeData.MaxChargeDuration;
	if (Data.ChargeData.TimeoutDuration>0.f)	TimeoutDuration = Data.ChargeData.TimeoutDuration;
	if (Data.ChargeData.CooldownDuration>0.f)	CooldownDuration = Data.ChargeData.CooldownDuration;
}

void USkillBehavior_Charge::OnChargeEventReceived(FGameplayEventData EventData)
{
	SetMontagePlayRate(0.01f);
}

void USkillBehavior_Charge::OnMaxCharged()
{
	if (bIsEnd)
		return;
	bIsEnd = true;
	CachedChargeDuration=MaxChargeDuration;
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateTimerHandle);
	if (PlayerCharacter)
		PlayerCharacter->OnChargeAbilityEnded.Broadcast();

	OwningAbility->ApplyDefaultCooldownOnce();
	SetMontagePlayRate(1.f);
}

void USkillBehavior_Charge::OnChargeReleased(float Time)
{
	
	if (bIsEnd)
		return;
	GetWorld()->GetTimerManager().ClearTimer(ChargeUpdateTimerHandle);
	if (PlayerCharacter)
		PlayerCharacter->OnChargeAbilityEnded.Broadcast();
	OwningAbility->GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetMoveBlockTag());
	bIsEnd = true;

	if (Time <= 0.2f)
	{
		OwningAbility->ApplyShortCooldownAndRequestEndAbility();
		return;
	}
	CachedChargeDuration = Time;
	OwningAbility->ApplyDefaultCooldownOnce();
	SetMontagePlayRate(1.f);
}

void USkillBehavior_Charge::HitTarget(FGameplayEventData EventData)
{
	TArray<FHitResult> HitResults = OwningAbility->GetHitResultFromVirtualSocketTargetData(EventData.TargetData);
	OwningAbility->ApplyDamageToHitResults(HitResults);
}

void USkillBehavior_Charge::UpdateChargeUI()
{
	if (PlayerCharacter)
	{
		// 경과 시간을 기반으로 충전 진행률(0.0 ~ 1.0) 계산
		const float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;
		const float ChargePercentage = FMath::Clamp(ElapsedTime / MaxChargeDuration, 0.0f, 1.0f);
        
		// UI에게 현재 진행률 방송
		PlayerCharacter->OnChargeAbilityUpdate.Broadcast(ChargePercentage);
	}
}
