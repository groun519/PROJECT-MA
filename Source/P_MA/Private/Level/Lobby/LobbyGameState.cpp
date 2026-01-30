// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameState.h"
#include "LobbyAvatarSlot.h"
#include "Player/MAPlayerState.h"
#include "Net/UnrealNetwork.h"

void ALobbyGameState::RegisterAvatarSlot(ALobbyAvatarSlot* Slot)
{
	if (!Slot) return;
	AvatarSlots.AddUnique(Slot);
}

void ALobbyGameState::AssignSlotToPlayer(AMAPlayerState* PlayerState)
{
	if (!PlayerState) return;

	const int32 SlotCount = AvatarSlots.Num();
	if (LobbySlots.Num() != SlotCount)
	{
		LobbySlots.SetNum(SlotCount);
	}

	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		if (!LobbySlots[Index].PlayerState)
		{
			LobbySlots[Index].PlayerState = PlayerState;
			LobbySlots[Index].bReady = false;
			if (AvatarSlots[Index])
			{
				AvatarSlots[Index]->SetOccupant(PlayerState);
			}
			return;
		}
	}
}

void ALobbyGameState::RemovePlayerFromSlot(AMAPlayerState* PlayerState)
{
	if (!PlayerState) return;
	for (int32 Index = 0; Index < LobbySlots.Num(); ++Index)
	{
		if (LobbySlots[Index].PlayerState == PlayerState)
		{
			LobbySlots[Index].PlayerState = nullptr;
			LobbySlots[Index].bReady = false;
			if (AvatarSlots.IsValidIndex(Index) && AvatarSlots[Index])
			{
				AvatarSlots[Index]->SetOccupant(nullptr);
			}
			return;
		}
	}
}

void ALobbyGameState::SetPlayerReady(APlayerState* PlayerState, bool bReady)
{
	if (!PlayerState) return;
	for (FPlayerLobbySlot& Slot : LobbySlots)
	{
		if (Slot.PlayerState == PlayerState)
		{
			Slot.bReady = bReady;
			return;
		}
	}
}

bool ALobbyGameState::IsPlayerReady(const APlayerState* PlayerState) const
{
	if (!PlayerState) return false;
	for (const FPlayerLobbySlot& Slot : LobbySlots)
	{
		if (Slot.PlayerState == PlayerState)
		{
			return Slot.bReady;
		}
	}
	return false;
}

int32 ALobbyGameState::GetSlotIndex(const APlayerState* PlayerState) const
{
	if (!PlayerState) return INDEX_NONE;
	for (int32 Index = 0; Index < LobbySlots.Num(); ++Index)
	{
		if (LobbySlots[Index].PlayerState == PlayerState)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

int32 ALobbyGameState::GetReadyCount() const
{
	int32 ReadyCount = 0;
	for (const FPlayerLobbySlot& Slot : LobbySlots)
	{
		if (Slot.PlayerState && Slot.bReady)
		{
			ReadyCount++;
		}
	}
	return ReadyCount;
}

int32 ALobbyGameState::GetPlayerCount() const
{
	return PlayerArray.Num();
}

void ALobbyGameState::OnRep_ReadyStates()
{
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, LobbySlots);
}
