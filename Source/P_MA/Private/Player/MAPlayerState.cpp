// Fill out your copyright notice in the Description page of Project Settings.

#include "MAPlayerState.h"
#include "Net/UnrealNetwork.h"

void AMAPlayerState::SetDefaultSkill(TSubclassOf<UGameplayAbility> NewSkill)
{
	DefaultSkill = NewSkill;
}

void AMAPlayerState::OnRep_DefaultSkill()
{
}

void AMAPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMAPlayerState, DefaultSkill);
}
