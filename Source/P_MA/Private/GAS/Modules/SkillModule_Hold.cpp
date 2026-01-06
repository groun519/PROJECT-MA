// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Hold.h"

#include "GAS/Ability/MAGameplayAbility_Skill.h"

void USkillModule_Hold::OnAbilityActivated()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill)	return;

	const FSkillData& SkillData = Skill->GetSkillData();
	if (!SkillData.SkillMontage)
	{
		Skill->EndAbility(Skill->GetCurrentAbilitySpecHandle(), Skill->GetCurrentActorInfo(), Skill->GetCurrentActivationInfo(), true, false);
		return;
	}

	bIsHolding = true;

	StartMontageTask();
	StartWaitJumpSectionEventTask();
	StartWaitInputReleaseTask();
	StartMaxHoldDelayTask();

	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		StartWaitDamageEventTask(FName("Event.Montage.Damage"));
	}
}

void USkillModule_Hold::OnAbilityEnded(bool bWasCancelled)
{
	if (MontageTask)				MontageTask->EndTask();
	if (InputReleaseTask)			InputReleaseTask->EndTask();
	if (JumpMontageSectionTask)		JumpMontageSectionTask->EndTask();
	if (DamageEventTask)			DamageEventTask->EndTask();
	if (MaxHoldTask)				MaxHoldTask->EndTask();
}


void USkillModule_Hold::StartMontageTask()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	const FSkillData& SkillData = Skill->GetSkillData();

	float PlayRate = Skill->GetTotalAnimSpeed();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(Skill, NAME_None, SkillData.SkillMontage,PlayRate,NAME_None,false);
	MontageTask->OnCompleted.AddDynamic(this, &USkillModule_Hold::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &USkillModule_Hold::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &USkillModule_Hold::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void USkillModule_Hold::OnMontageEnded()
{
	if (UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill))
	{
		Skill->EndAbility(Skill->GetCurrentAbilitySpecHandle(), Skill->GetCurrentActorInfo(), Skill->GetCurrentActivationInfo(), true, false);
	}
}

void USkillModule_Hold::StartWaitDamageEventTask(FName TagName)
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(TagName);

	DamageEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(Skill,EventTag,nullptr,false,true);
	DamageEventTask->EventReceived.AddDynamic(this, &USkillModule_Hold::OnDamageEventReceived);
	DamageEventTask->ReadyForActivation();
}

void USkillModule_Hold::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill))
	{
		Skill->ExecuteSkillAction(Payload, 0.8f);
	}
}

void USkillModule_Hold::StartWaitJumpSectionEventTask()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	FGameplayTag Tag = FGameplayTag::RequestGameplayTag("Event.Montage.JumpSection");

	JumpMontageSectionTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(Skill, Tag);
	JumpMontageSectionTask -> EventReceived.AddDynamic(this, &USkillModule_Hold::OnJumpSectionEventReceived);
	JumpMontageSectionTask -> ReadyForActivation();
}

void USkillModule_Hold::OnJumpSectionEventReceived(FGameplayEventData Payload)
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill)	return;

	Skill -> Montage_SetSection(FName("LoopStart"));
}

void USkillModule_Hold::StartWaitInputReleaseTask()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(Skill);
	InputReleaseTask -> OnRelease.AddDynamic(this, &USkillModule_Hold::OnInputRelease);
	InputReleaseTask -> ReadyForActivation();
}

void USkillModule_Hold::OnInputRelease(float TimeHeld)
{
	if (!bIsHolding)	return;

	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill)	return;

	Skill -> Montage_SetSection(FName("LoopEnd"));

	bIsHolding = false;
	if (MaxHoldTask)
			MaxHoldTask->EndTask();
}

void USkillModule_Hold::StartMaxHoldDelayTask()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);

	MaxHoldTask = UAbilityTask_WaitDelay::WaitDelay(Skill, 2.5f);
	MaxHoldTask -> OnFinish.AddDynamic(this, &USkillModule_Hold::OnMaxHold);
	MaxHoldTask -> ReadyForActivation();
}

void USkillModule_Hold::OnMaxHold()
{
	if (!bIsHolding)	return;

	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill)	return;

	Skill -> Montage_SetSection(FName("LoopEnd"));

	bIsHolding = false;
	if (InputReleaseTask)
		InputReleaseTask->EndTask();
}
