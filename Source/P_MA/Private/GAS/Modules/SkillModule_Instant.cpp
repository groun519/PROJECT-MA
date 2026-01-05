// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Modules/SkillModule_Instant.h"

#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"

void USkillModule_Instant::OnAbilityActivated()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	if (!Skill)	return;

	const FSkillData& SkillData = Skill->GetSkillData();
	if (!SkillData.SkillMontage)
	{
		Skill->EndAbility(Skill->GetCurrentAbilitySpecHandle(), Skill->GetCurrentActorInfo(), Skill->GetCurrentActivationInfo(), true, false);
		return;
	}
	
	StartMontageTask();
	if (SkillData.ActionTags.HasTag(FGameplayTag::RequestGameplayTag("Ability.Action.Melee")))
	{
		StartWaitForEventTask(FName("Event.Montage.Damage"));
	}
}

void USkillModule_Instant::OnAbilityEnded(bool bWasCancelled)
{
	if (MontageTask)	MontageTask->EndTask();
	if (EventTask)		EventTask->EndTask();
}

void USkillModule_Instant::StartMontageTask()
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	const FSkillData& SkillData = Skill->GetSkillData();

	float PlayRate = Skill->GetTotalAnimSpeed();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(Skill,NAME_None,SkillData.SkillMontage,PlayRate,NAME_None,false);
	MontageTask->OnCompleted.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->OnInterrupted.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->OnBlendOut.AddDynamic(this, &USkillModule_Instant::OnMontageEnded);
	MontageTask->ReadyForActivation();
}

void USkillModule_Instant::OnMontageEnded()
{
	if (UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill))
	{
		Skill->EndAbility(Skill->GetCurrentAbilitySpecHandle(), Skill->GetCurrentActorInfo(), Skill->GetCurrentActivationInfo(), true, false);
	}
}

void USkillModule_Instant::StartWaitForEventTask(FName TagName)
{
	UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill);
	FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(TagName);

	EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(Skill,EventTag,nullptr,false,true);
	EventTask->EventReceived.AddDynamic(this, &USkillModule_Instant::OnEventReceived);
	EventTask->ReadyForActivation();
}

void USkillModule_Instant::OnEventReceived(FGameplayEventData Payload)
{
	if (UMAGameplayAbility_Skill* Skill = Cast<UMAGameplayAbility_Skill>(OwnerSkill))
	{
		Skill->ExecuteSkillAction(Payload, 1.f);
	}
}

