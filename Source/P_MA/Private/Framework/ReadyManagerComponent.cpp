// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/ReadyManagerComponent.h"
#include "Framework/MAGameMode.h"
#include "Framework/MAGameState.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/Components/ReadyStateComponent.h"

namespace
{
	UReadyStateComponent* ResolveReadyComponent(AMAPlayerCharacter* Player)
	{
		return Player ? Player->GetReadyStateComponent() : nullptr;
	}

	const UReadyStateComponent* ResolveReadyComponent(const AMAPlayerCharacter* Player)
	{
		return Player ? Player->GetReadyStateComponent() : nullptr;
	}
}

UReadyManagerComponent::UReadyManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UReadyManagerComponent::RefreshPlayerCache()
{
	CachedPlayers.Reset();

	UWorld* World = GetWorld();
	if (!World)
	{
		BroadcastReadyCounts();
		return;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(PC->GetPawn());
		if (!Player) continue;

		CachedPlayers.Add(Player);
	}
	SyncLoopReadyToGameState();
	BroadcastReadyCounts();
}

void UReadyManagerComponent::ResetAllPlayersReady()
{
	for (TWeakObjectPtr<AMAPlayerCharacter> PlayerPtr : CachedPlayers)
	{
		if (UReadyStateComponent* ReadyComp = ResolveReadyComponent(PlayerPtr.Get()))
		{
			ReadyComp->SetReady(false);
		}
	}

	BroadcastReadyCounts();
}

void UReadyManagerComponent::GetReadyCounts(int32& OutReady, int32& OutTotal) const
{
	OutReady = 0;
	OutTotal = 0;

	const AMAGameState* GS = GetMAGameState();
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS) continue;

		OutTotal++;
		if (const UReadyStateComponent* ReadyComp = FindReadyComponentByPlayerState(PS))
		{
			if (ReadyComp->IsReady())
			{
				OutReady++;
			}
		}
	}
}

void UReadyManagerComponent::BroadcastReadyCounts()
{
	int32 ReadyCount = 0;
	int32 TotalCount = 0;
	GetReadyCounts(ReadyCount, TotalCount);

	const bool bPrevAllReady = bAllPlayersReady;
	bAllPlayersReady = (TotalCount > 0 && ReadyCount == TotalCount);

	AMAGameMode* OwningGM = Cast<AMAGameMode>(GetOwner());
	if (OwningGM && !bPrevAllReady && bAllPlayersReady)
	{
		OwningGM->RequestNextState(OwningGM->GetMASectorState());
	}

	OnReadyCountsChanged.Broadcast(ReadyCount, TotalCount);
	if (bPrevAllReady != bAllPlayersReady)
	{
		OnAllPlayersReadyChanged.Broadcast(bAllPlayersReady);
	}
}

void UReadyManagerComponent::SetPlayerLoopReady(APlayerState* PlayerState, bool bReady)
{
	if (!PlayerState) return;

	if (UReadyStateComponent* ReadyComp = FindReadyComponentByPlayerState(PlayerState))
	{
		ReadyComp->SetLoopReady(bReady);
	}

	SyncLoopReadyToGameState();

	int32 ReadyCount = 0;
	int32 TotalCount = 0;
	GetLoopReadyCounts(ReadyCount, TotalCount);

	const bool bIsAllReady = (TotalCount > 0 && ReadyCount == TotalCount);
	AMAGameMode* OwningGM = Cast<AMAGameMode>(GetOwner());
	if (OwningGM && bIsAllReady && OwningGM->GetMASectorState() == EMASectorState::Loop)
	{
		OwningGM->RequestStateChange(EMASectorState::Start);
		ResetAllLoopReady();
	}
}

bool UReadyManagerComponent::IsPlayerLoopReady(const APlayerState* PlayerState) const
{
	if (!PlayerState) return false;

	if (const UReadyStateComponent* ReadyComp = FindReadyComponentByPlayerState(PlayerState))
	{
		return ReadyComp->IsLoopReady();
	}

	const AMAGameState* GS = GetMAGameState();
	return GS ? GS->GetLoopReadyForPlayer(PlayerState) : false;
}

AMAGameState* UReadyManagerComponent::GetMAGameState() const
{
	const AMAGameMode* OwningGM = Cast<AMAGameMode>(GetOwner());
	if (!OwningGM) return nullptr;

	return OwningGM->GetGameState<AMAGameState>();
}

AMAPlayerCharacter* UReadyManagerComponent::FindPlayerCharacterByPlayerState(const APlayerState* PlayerState) const
{
	if (!PlayerState) return nullptr;

	for (TWeakObjectPtr<AMAPlayerCharacter> PlayerPtr : CachedPlayers)
	{
		AMAPlayerCharacter* Player = PlayerPtr.Get();
		if (Player && Player->GetPlayerState() == PlayerState)
		{
			return Player;
		}
	}

	APawn* Pawn = PlayerState->GetPawn();
	return Cast<AMAPlayerCharacter>(Pawn);
}

UReadyStateComponent* UReadyManagerComponent::FindReadyComponentByPlayerState(const APlayerState* PlayerState)
{
	return ResolveReadyComponent(FindPlayerCharacterByPlayerState(PlayerState));
}

const UReadyStateComponent* UReadyManagerComponent::FindReadyComponentByPlayerState(const APlayerState* PlayerState) const
{
	return ResolveReadyComponent(FindPlayerCharacterByPlayerState(PlayerState));
}

void UReadyManagerComponent::SyncLoopReadyToGameState() const
{
	AMAGameState* GS = GetMAGameState();
	if (!GS) return;

	GS->SyncLoopReadyEntries(GS->PlayerArray);
	for (APlayerState* PS : GS->PlayerArray)
	{
		const UReadyStateComponent* ReadyComp = FindReadyComponentByPlayerState(PS);
		GS->SetLoopReadyForPlayer(PS, ReadyComp ? ReadyComp->IsLoopReady() : false);
	}
}

void UReadyManagerComponent::GetLoopReadyCounts(int32& OutReady, int32& OutTotal) const
{
	OutReady = 0;
	OutTotal = 0;

	const AMAGameState* GS = GetMAGameState();
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (!PS) continue;

		OutTotal++;
		OutReady += IsPlayerLoopReady(PS) ? 1 : 0;
	}
}

void UReadyManagerComponent::ResetAllLoopReady()
{
	const AMAGameState* GS = GetMAGameState();
	if (!GS) return;

	for (APlayerState* PS : GS->PlayerArray)
	{
		if (UReadyStateComponent* ReadyComp = FindReadyComponentByPlayerState(PS))
		{
			ReadyComp->SetLoopReady(false);
		}
	}

	SyncLoopReadyToGameState();
}
