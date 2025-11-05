// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MASkillBehavior.h"
#include "MAGameplayAbility_SkillBase.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "GameFramework/PlayerController.h"
#include "Player/MAPlayerCharacter.h"

void UMASkillBehavior::OnActivate_Implementation()
{
	OwningAbility->IgnoreTargets.Empty();
	this->Character = GetCharacter();
	this->PlayerCharacter = Cast<AMAPlayerCharacter>(this->Character);
}

void UMASkillBehavior::OnEndAbility_Implementation()
{
	this->Character = nullptr;
	this->PlayerCharacter = nullptr;
}

void UMASkillBehavior::ApplyCooldownAndEndAbility(TSubclassOf<UGameplayEffect> CooldownEffect)
{
	if (!OwningAbility)
		return;
	if (CooldownEffect)
	{
		OwningAbility->ApplyEffectToOwner(CooldownEffect);
	}
	UAbilityTask_WaitDelay* EndDelayTask = UAbilityTask_WaitDelay::WaitDelay(OwningAbility, 0.05f);
	EndDelayTask->OnFinish.AddDynamic(this, &UMASkillBehavior::SafeEndAbility);
	EndDelayTask->ReadyForActivation();
}


void UMASkillBehavior::SafeEndAbility()
{
	if (OwningAbility)
		OwningAbility->RequestEndAbility();
}

class AMACharacter* UMASkillBehavior::GetCharacter() const
{
	if (OwningAbility)
	{
		return Cast<AMACharacter>(OwningAbility->GetAvatarActorFromActorInfo());
	}
	return nullptr;
}
