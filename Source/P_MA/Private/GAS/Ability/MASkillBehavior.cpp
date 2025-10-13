// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/MASkillBehavior.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MAPlayerCharacter.h"

void UMASkillBehavior::OnActivate_Implementation()
{
	this->PlayerCharacter = GetPlayerCharacter();
}

void UMASkillBehavior::OnEndAbility_Implementation()
{
	this->PlayerCharacter = nullptr;
}

class AMAPlayerCharacter* UMASkillBehavior::GetPlayerCharacter()
{
	ACharacter* FoundCharacter = UGameplayStatics::GetPlayerCharacter(this,0);
	return Cast<AMAPlayerCharacter>(FoundCharacter);
}
