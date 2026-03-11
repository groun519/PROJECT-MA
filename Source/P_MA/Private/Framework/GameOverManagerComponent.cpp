// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/GameOverManagerComponent.h"
#include "Framework/MAGameMode.h"
#include "Framework/MAGameState.h"
#include "GameFramework/PlayerState.h"
#include "Player/MAPlayerCharacter.h"

UGameOverManagerComponent::UGameOverManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGameOverManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AMAGameMode* GM = GetOwnerGameMode())
	{
		GM->OnMASectorStateChanged.RemoveAll(this);
		GM->OnMASectorStateChanged.AddUObject(this, &UGameOverManagerComponent::HandleSectorStateChanged);
	}
}

void UGameOverManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AMAGameMode* GM = GetOwnerGameMode())
	{
		GM->OnMASectorStateChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UGameOverManagerComponent::TryTriggerGameOver()
{
	// TODO: Implement game over evaluation logic.
}

void UGameOverManagerComponent::HandleSectorStateChanged(EMASectorState NewState)
{
	if (NewState == EMASectorState::EndBattle)
	{
		ReviveAllDeadPlayers();
	}
}

void UGameOverManagerComponent::ReviveAllDeadPlayers()
{
	AMAGameState* GS = GetMAGameState();
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS) continue;

		AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(PS->GetPawn());
		if (!PlayerCharacter) continue;

		if (PlayerCharacter->IsDead())
		{
			PlayerCharacter->RespawnImmediately();
		}
	}
}

AMAGameMode* UGameOverManagerComponent::GetOwnerGameMode() const
{
	return Cast<AMAGameMode>(GetOwner());
}

AMAGameState* UGameOverManagerComponent::GetMAGameState() const
{
	const AMAGameMode* GM = GetOwnerGameMode();
	if (!GM) return nullptr;

	return GM->GetGameState<AMAGameState>();
}
