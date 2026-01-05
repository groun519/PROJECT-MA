// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Charge.h"

#include "GAS/Ability/MAGameplayAbility_Skill.h"

void USkillModule_Charge::OnAbilityActivated()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill)	return;

	const FSkillData& SkillData = Skill->GetSkillData();
	if (!SkillData.SkillMontage)
	{
		Skill->EndAbility(Skill->GetCurrentAbilitySpecHandle(), Skill->GetCurrentActorInfo(), Skill->GetCurrentActivationInfo(), true, false);
		return;
	}

	ChargeStartTime = 0.f;
	FinalChargedDuration = 0.f;
	bIsCharging = false;

	StartMontageTask();
	StartChargeTask();

	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		StartWaitForEventTask(FName("Event.Montage.Damage"));
	}
}

void USkillModule_Charge::OnAbilityEnded(bool bWasCancelled)
{
	if (MontageTask)			MontageTask->EndTask();
	if (InputReleaseTask)		InputReleaseTask->EndTask();
	if (ChargeStartEventTask)	ChargeStartEventTask->EndTask();
	if (EventTask)				EventTask->EndTask();
	if (MaxChargeTask)			MaxChargeTask->EndTask();
}

void USkillModule_Charge::StartMontageTask()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	const FSkillData& SkillData = Skill->GetSkillData();

	float PlayRate = Skill->GetTotalAnimSpeed();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(Skill,NAME_None,SkillData.SkillMontage,PlayRate,NAME_None,false);
	MontageTask->OnCompleted.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &USkillModule_Charge::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void USkillModule_Charge::OnMontageEnded()
{
	if (UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill))
	{
		Skill->EndAbility(Skill->GetCurrentAbilitySpecHandle(), Skill->GetCurrentActorInfo(), Skill->GetCurrentActivationInfo(), true, false);
	}
}

void USkillModule_Charge::StartChargeTask()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Event.Montage.SlowPlay");

	ChargeStartEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(Skill, Tag, nullptr, false, true);
	ChargeStartEventTask -> EventReceived.AddDynamic(this, &USkillModule_Charge::OnChargeEventReceived);
	ChargeStartEventTask -> ReadyForActivation();
}

void USkillModule_Charge::OnChargeEventReceived(FGameplayEventData Payload)
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill) return;
	
	if (UAnimMontage* Montage = Skill->GetCurrentMontage())
	{
		Skill->Montage_SetPlayRate(Montage, 0.01f);
	}
	bIsCharging = true;
	ChargeStartTime = GetWorld()->GetTimeSeconds();

	StartWaitInputReleaseTask();
	StartMaxChargeDelayTask();
}

void USkillModule_Charge::StartWaitForEventTask(FName TagName)
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(TagName);

	EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(Skill,EventTag,nullptr,false,true);
	EventTask->EventReceived.AddDynamic(this, &USkillModule_Charge::OnEventReceived);
	EventTask->ReadyForActivation();
}

void USkillModule_Charge::OnEventReceived(FGameplayEventData Payload)
{
	if (UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill))
	{
		Skill->ExecuteSkillAction(Payload, FinalChargedDuration);
	}
}

void USkillModule_Charge::StartWaitInputReleaseTask()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	
	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(Skill);
	InputReleaseTask -> OnRelease.AddDynamic(this, &USkillModule_Charge::OnInputReleased);
	InputReleaseTask -> ReadyForActivation();
}

void USkillModule_Charge::OnInputReleased(float TimeHeld)
{
	if (!bIsCharging)	return;

	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);

	FinalChargedDuration = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	
	if (Skill && Skill->GetCurrentMontage())
	{
		Skill->Montage_SetPlayRate(Skill->GetCurrentMontage(), 1.0f);
	}
	
	bIsCharging = false;
	if (MaxChargeTask)
		MaxChargeTask->EndTask();
}

void USkillModule_Charge::StartMaxChargeDelayTask()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);

	MaxChargeTask = UAbilityTask_WaitDelay::WaitDelay(Skill, 3.f);
	MaxChargeTask -> OnFinish.AddDynamic(this, &USkillModule_Charge::OnMaxCharged);
	MaxChargeTask -> ReadyForActivation();
}

void USkillModule_Charge::OnMaxCharged()
{
	if (!bIsCharging)	return;

	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	
	const FSkillData& Data = Skill->GetSkillData();
	FinalChargedDuration = 3.f;
	
	if (Skill && Skill->GetCurrentMontage())
	{
		Skill->Montage_SetPlayRate(Skill->GetCurrentMontage(), 1.0f);
	}

	bIsCharging = false;
	if (InputReleaseTask) InputReleaseTask->EndTask();
}
