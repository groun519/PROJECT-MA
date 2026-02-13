// Fill out your copyright notice in the Description page of Project Settings.

#include "Framework/MAGameState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerState.h"

AMAGameState::AMAGameState()
{
}

void AMAGameState::SetMAGameState(EMAGameState NewState)
{
	if (ReplicatedState == NewState)
	{
		return;
	}

	ReplicatedState = NewState;
	OnMAGameStateChanged.Broadcast(ReplicatedState);
}

void AMAGameState::OnRep_MAGameState()
{
	OnMAGameStateChanged.Broadcast(ReplicatedState);
}

void AMAGameState::SyncLoopReadyEntries(const TArray<APlayerState*>& Players)
{
	bool bChanged = false;

	// Remove entries for players no longer present
	for (int32 Index = LoopReadyEntries.Num() - 1; Index >= 0; --Index)
	{
		if (!LoopReadyEntries[Index].PlayerState || !Players.Contains(LoopReadyEntries[Index].PlayerState))
		{
			LoopReadyEntries.RemoveAt(Index);
			bChanged = true;
		}
	}

	// Add entries for new players
	for (APlayerState* PS : Players)
	{
		if (!PS)
		{
			continue;
		}

		bool bFound = false;
		for (const FLoopReadyEntry& Entry : LoopReadyEntries)
		{
			if (Entry.PlayerState == PS)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			FLoopReadyEntry NewEntry;
			NewEntry.PlayerState = PS;
			NewEntry.bReady = false;
			LoopReadyEntries.Add(NewEntry);
			bChanged = true;
		}
	}

	if (bChanged)
	{
		OnLoopReadyEntriesChanged.Broadcast();
	}
}

void AMAGameState::SetLoopReadyForPlayer(APlayerState* PlayerState, bool bReady)
{
	if (!PlayerState)
	{
		return;
	}

	for (FLoopReadyEntry& Entry : LoopReadyEntries)
	{
		if (Entry.PlayerState == PlayerState)
		{
			if (Entry.bReady == bReady)
			{
				return;
			}
			Entry.bReady = bReady;
			OnLoopReadyEntriesChanged.Broadcast();
			return;
		}
	}

	FLoopReadyEntry NewEntry;
	NewEntry.PlayerState = PlayerState;
	NewEntry.bReady = bReady;
	LoopReadyEntries.Add(NewEntry);
	OnLoopReadyEntriesChanged.Broadcast();
}

bool AMAGameState::GetLoopReadyForPlayer(const APlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return false;
	}

	for (const FLoopReadyEntry& Entry : LoopReadyEntries)
	{
		if (Entry.PlayerState == PlayerState)
		{
			return Entry.bReady;
		}
	}
	return false;
}

void AMAGameState::GetLoopReadyCounts(int32& OutReady, int32& OutTotal) const
{
	OutReady = 0;
	OutTotal = LoopReadyEntries.Num();
	for (const FLoopReadyEntry& Entry : LoopReadyEntries)
	{
		if (Entry.bReady)
		{
			OutReady++;
		}
	}
}

void AMAGameState::ResetLoopReadyEntries()
{
	bool bChanged = false;
	for (FLoopReadyEntry& Entry : LoopReadyEntries)
	{
		if (Entry.bReady)
		{
			Entry.bReady = false;
			bChanged = true;
		}
	}
	if (bChanged)
	{
		OnLoopReadyEntriesChanged.Broadcast();
	}
}

void AMAGameState::OnRep_LoopReadyEntries()
{
	OnLoopReadyEntriesChanged.Broadcast();
}

void AMAGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAGameState, ReplicatedState);
	DOREPLIFETIME(AMAGameState, LoopReadyEntries);
}
