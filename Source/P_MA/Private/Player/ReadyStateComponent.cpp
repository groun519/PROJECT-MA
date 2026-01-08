// Fill out your copyright notice in the Description page of Project Settings.


#include "ReadyStateComponent.h"
#include "MAPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "P_MA/P_MA.h"

UReadyStateComponent::UReadyStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UReadyStateComponent::SetReady(bool bNewReady)
{
	bIsReady = bNewReady;

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(GetOwner());
	if (!PlayerCharacter) return;
	
	PlayerCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(
		ECC_ReadyWall,
		IsReady() ? ECR_Block : ECR_Overlap
		);
}
