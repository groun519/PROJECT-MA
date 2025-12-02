// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/SkillBehavior_Chain.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAGameplayAbility.h"
#include "GameplayTagsManager.h"
#include "SkillBehaviorConfig.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"


void USkillBehavior_Chain::OnActivate_Implementation()
{
	if (!OwningAbility)
		return;
	Super::OnActivate_Implementation();
	
	bIsComboInputBuffered = false;

	WaitComboChangeEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility,ComboChangeEventTag,nullptr,false,false);
	WaitComboChangeEventTask->EventReceived.AddDynamic(this, &USkillBehavior_Chain::ComboChangedEventReceived);
	WaitComboChangeEventTask->ReadyForActivation();
	
	WaitClearEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(OwningAbility, IgnoreClearTag);
	WaitClearEventTask->EventReceived.AddDynamic(this, &USkillBehavior_Chain::ClearIgnore);
	WaitClearEventTask->ReadyForActivation();
	
	SetupWaitComboInputPress();
}

void USkillBehavior_Chain::OnEndAbility_Implementation()
{
	if (WaitComboChangeEventTask.IsValid())
		WaitComboChangeEventTask->EndTask();
	if (WaitClearEventTask.IsValid())
		WaitClearEventTask->EndTask();
	if (WaitInputPress.IsValid())
		WaitInputPress->EndTask();
	
	Super::OnEndAbility_Implementation();
}


void USkillBehavior_Chain::SetupWaitComboInputPress()
{
	WaitInputPress = UAbilityTask_WaitInputPress::WaitInputPress(OwningAbility);
	WaitInputPress->OnPress.AddDynamic(this, &USkillBehavior_Chain::HandleInputPress);
	WaitInputPress->ReadyForActivation();
}

void USkillBehavior_Chain::ComboChangedEventReceived(FGameplayEventData EventData)
{
	FGameplayTag EventTag = EventData.EventTag;
	if (EventTag == ComboEndEventTag)
		return;
	
	TArray<FName> TagNames;
	UGameplayTagsManager::Get().SplitGameplayTagFName(EventTag,TagNames);
	NextComboName = TagNames.Last();
}

void USkillBehavior_Chain::ClearIgnore(FGameplayEventData EventData)
{
	if (OwningAbility->K2_HasAuthority())
	{
		OwningAbility->IgnoreTargets.Empty();
	}
	if (bIsComboInputBuffered && NextComboName != NAME_None)
	{
		MontageToOtherSection(NextComboName);
	}
	
	bIsComboInputBuffered = false;
	NextComboName = NAME_None;
}

void USkillBehavior_Chain::HandleInputPress(float Time)
{
	bIsComboInputBuffered = true;
	SetupWaitComboInputPress();
}

void USkillBehavior_Chain::TryCommitCombo()
{
	if (NextComboName == NAME_None)
		return;

	UAnimInstance* OwnerAnimInst = OwningAbility->GetOwnerAnimInstance();
	if (!OwnerAnimInst)
		return;

	OwnerAnimInst->Montage_SetNextSection(OwnerAnimInst->Montage_GetCurrentSection(MontageToPlay), NextComboName, MontageToPlay);
}

float USkillBehavior_Chain::GetDamageMultiplierForCurrentCombo() const
{
	UAnimInstance* OwnerAnimInst = OwningAbility->GetOwnerAnimInstance();
	if (OwnerAnimInst)
	{
		FName CurrentSectionName = OwnerAnimInst->Montage_GetCurrentSection(MontageToPlay);
		const float* FoundMultiplier = DamageMultiplierMap.Find(CurrentSectionName);
		if (FoundMultiplier)
			return *FoundMultiplier;
	}
	return BehaviorDamageMultiplier;
}

float USkillBehavior_Chain::GetCurrentDamageMultiplier() const
{
	return GetDamageMultiplierForCurrentCombo();
}

void USkillBehavior_Chain::InitFromConfig(const FInstancedStruct& ConfigPayload)
{
	Super::InitFromConfig(ConfigPayload);
	const FConfig_Chain* ChainConfig = ConfigPayload.GetPtr<FConfig_Chain>();
	if (ChainConfig)
	{
		DamageMultiplierMap = ChainConfig->DamageMultiplierMap;
	}
}
