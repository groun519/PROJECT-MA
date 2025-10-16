// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MASkillBehavior.h"
#include "MAGameplayAbility_SkillBase.h"
#include "Player/MAPlayerCharacter.h"

void UMASkillBehavior::OnActivate_Implementation()
{
	this->Character = GetCharacter();
	this->PlayerCharacter = Cast<AMAPlayerCharacter>(this->Character);
}

void UMASkillBehavior::OnEndAbility_Implementation()
{
	this->Character = nullptr;
	this->PlayerCharacter = nullptr;
}

class AMACharacter* UMASkillBehavior::GetCharacter() const
{
	if (OwningAbility)
	{
		return Cast<AMACharacter>(OwningAbility->GetAvatarActorFromActorInfo());
	}
	return nullptr;
}
