// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Charge.h"
#include "GAS/Modules/MASkillModuleData.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"

void USkillModule_Charge::OnAbilityActivated()
{
	if (!OwnerSkill)	return;

	const FSkillData& SkillData = OwnerSkill->GetSkillData();
	if (!SkillData.SkillMontage)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
		return;
	}
	
	FinalChargedDuration = 0.f;
	bIsCharging = false;

	CachedMaxChargeDuration = 3.f;
	CachedMaxInputDelay = 3.5f;
	
	const FModuleBehaviorData& BehaviorData = OwnerSkill->GetBehaviorData();
	if (const FBehavior_Charge* Config = BehaviorData.ModuleConfig.GetPtr<FBehavior_Charge>())
	{
		CachedMaxChargeDuration = Config->MaxChargeDuration;
		CachedMaxInputDelay = Config->MaxInputDelay;
	}
	
	//몽타주 재생 및 애니메이션 속도 늦추도록
	StartMontageTask();
	StartChargeTask();

	//공격 방식에 따라 다르게 데미지 적용 (근접/타게팅/투사체)
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		StartWaitDamageEventTask(FName("Event.Montage.Damage"));
	}
}

void USkillModule_Charge::OnAbilityEnded(bool bWasCancelled)
{
	if (MontageTask)			MontageTask->EndTask();
	if (InputReleaseTask)		InputReleaseTask->EndTask();
	if (ChargeStartEventTask)	ChargeStartEventTask->EndTask();
	if (DamageEventTask)		DamageEventTask->EndTask();
	if (MaxChargeTask)			MaxChargeTask->EndTask();
}

void USkillModule_Charge::StartMontageTask()
{
	if (!OwnerSkill)	return;
	const FSkillData& SkillData = OwnerSkill->GetSkillData();

	float PlayRate = OwnerSkill->GetTotalAnimSpeed();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(OwnerSkill,NAME_None,SkillData.SkillMontage,PlayRate,NAME_None,false);
	MontageTask->OnCompleted.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void USkillModule_Charge::OnMontageEnded()
{
	if (OwnerSkill)
	{
		OwnerSkill->EndAbility(OwnerSkill->GetCurrentAbilitySpecHandle(), OwnerSkill->GetCurrentActorInfo(), OwnerSkill->GetCurrentActivationInfo(), true, false);
	}
}

void USkillModule_Charge::StartChargeTask()
{
	if (!OwnerSkill)	return;
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Event.Montage.SlowPlay");

	ChargeStartEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill, Tag, nullptr, false, true);
	ChargeStartEventTask -> EventReceived.AddDynamic(this, &USkillModule_Charge::OnChargeEventReceived);
	ChargeStartEventTask -> ReadyForActivation();
}

void USkillModule_Charge::OnChargeEventReceived(FGameplayEventData Payload)
{
	if (!OwnerSkill)	return;
	
	if (UAnimMontage* Montage = OwnerSkill->GetCurrentMontage())
	{
		OwnerSkill->Montage_SetPlayRate(Montage, 0.001f);
	}
	bIsCharging = true;
	FinalChargedDuration = 0.f;

	StartWaitInputReleaseTask();
	StartMaxChargeDelayTask();
}

void USkillModule_Charge::StartWaitDamageEventTask(FName TagName)
{
	if (!OwnerSkill)	return;
	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(TagName);

	DamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwnerSkill,EventTag,nullptr,false,true);
	DamageEventTask->EventReceived.AddDynamic(this, &USkillModule_Charge::OnDamageEventReceived);
	DamageEventTask->ReadyForActivation();
}

void USkillModule_Charge::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (OwnerSkill)
	{
		OwnerSkill->ExecuteSkillAction(Payload, FinalChargedDuration);
	}
}

void USkillModule_Charge::StartWaitInputReleaseTask()
{
	if (!OwnerSkill)	return;
	
	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(OwnerSkill);
	InputReleaseTask -> OnRelease.AddDynamic(this, &USkillModule_Charge::OnInputReleased);
	InputReleaseTask -> ReadyForActivation();
}

void USkillModule_Charge::OnInputReleased(float TimeHeld)
{
	if (!bIsCharging || !OwnerSkill)	return;
	
	
	FinalChargedDuration = FMath::Clamp(TimeHeld, 0.f, CachedMaxChargeDuration);
	
	if (OwnerSkill && OwnerSkill->GetCurrentMontage())
	{
		OwnerSkill->Montage_SetPlayRate(OwnerSkill->GetCurrentMontage(), 1.0f);
	}
	
	bIsCharging = false;
	if (MaxChargeTask)
		MaxChargeTask->EndTask();
}

void USkillModule_Charge::StartMaxChargeDelayTask()
{
	if (!OwnerSkill)	return;
	
	MaxChargeTask = UAbilityTask_WaitDelay::WaitDelay(OwnerSkill, CachedMaxInputDelay);
	MaxChargeTask -> OnFinish.AddDynamic(this, &USkillModule_Charge::OnMaxCharged);
	MaxChargeTask -> ReadyForActivation();
}

void USkillModule_Charge::OnMaxCharged()
{
	if (!bIsCharging || !OwnerSkill)	return;

	FinalChargedDuration = CachedMaxChargeDuration;
	if (OwnerSkill && OwnerSkill->GetCurrentMontage())
	{
		OwnerSkill->Montage_SetPlayRate(OwnerSkill->GetCurrentMontage(), 1.0f);
	}

	bIsCharging = false;
	if (InputReleaseTask)
		InputReleaseTask->EndTask();
}
