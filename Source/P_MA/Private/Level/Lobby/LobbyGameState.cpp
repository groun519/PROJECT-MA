// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameState.h"
#include "LobbyAvatarSlot.h"
#include "Player/MAPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"

void ALobbyGameState::RegisterAvatarSlot(ALobbyAvatarSlot* Slot)
{
	if (!Slot) return;
	const int32 Index = Slot->SlotIndex;
	if (Index < 0) return;

	if (AvatarSlots.Num() <= Index)
	{
		AvatarSlots.SetNum(Index + 1);
	}

	AvatarSlots[Index] = Slot;
	OnSlotsRegistered.Broadcast();
}

void ALobbyGameState::AssignSlotToPlayer(AMAPlayerState* PlayerState)
{
	if (!PlayerState) return;

	const int32 SlotCount = AvatarSlots.Num();
	if (LobbySlots.Num() != SlotCount)
	{
		LobbySlots.SetNum(SlotCount);
	}
	if (LobbyStates.Num() != SlotCount)
	{
		LobbyStates.SetNum(SlotCount);
	}

	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		if (!LobbySlots[Index].PlayerState)
		{
			LobbySlots[Index].PlayerState = PlayerState;
			LobbySlots[Index].bReady = false;
			LobbyStates[Index] = ELobbyAvatarState::Wait;
			PlayerState->SetLobbySlotIndex(Index);
			if (AvatarSlots[Index])
			{
				AvatarSlots[Index]->SetOccupant(PlayerState);
				AvatarSlots[Index]->SetLobbyState(ELobbyAvatarState::Wait);
			}
			if (APlayerController* PC = Cast<APlayerController>(PlayerState->GetOwner()))
			{
				if (PC->IsLocalController())
				{
					SetPlayerReady(PlayerState, true);
				}
			}
			ApplyLobbySlotsToAvatars();
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
			PlayerState->SetLobbySlotIndex(INDEX_NONE);
			LobbySlots[Index].PlayerState = nullptr;
			LobbySlots[Index].bReady = false;
			if (LobbyStates.IsValidIndex(Index))
			{
				LobbyStates[Index] = ELobbyAvatarState::Wait;
			}
			if (AvatarSlots.IsValidIndex(Index) && AvatarSlots[Index])
			{
				AvatarSlots[Index]->SetOccupant(nullptr);
				AvatarSlots[Index]->SetLobbyState(ELobbyAvatarState::Wait);
			}
			ApplyLobbySlotsToAvatars();
			return;
		}
	}
}

void ALobbyGameState::SetPlayerReady(APlayerState* PlayerState, bool bReady)
{
	if (!PlayerState) return;
	for (int32 Index = 0; Index < LobbySlots.Num(); ++Index)
	{
		FPlayerLobbySlot& Slot = LobbySlots[Index];
		if (Slot.PlayerState == PlayerState)
		{
			Slot.bReady = bReady;
			if (LobbyStates.IsValidIndex(Index) && LobbyStates[Index] != ELobbyAvatarState::Loadout)
			{
				LobbyStates[Index] = bReady ? ELobbyAvatarState::Ready : ELobbyAvatarState::Wait;
			}
			if (AvatarSlots.IsValidIndex(Index) && AvatarSlots[Index])
			{
				AvatarSlots[Index]->SetLobbyState(LobbyStates.IsValidIndex(Index) ? LobbyStates[Index] : ELobbyAvatarState::Wait);
			}
			return;
		}
	}
}

void ALobbyGameState::SetPlayerLobbyState(APlayerState* PlayerState, ELobbyAvatarState NewState)
{
	if (!PlayerState) return;
	for (int32 Index = 0; Index < LobbySlots.Num(); ++Index)
	{
		FPlayerLobbySlot& Slot = LobbySlots[Index];
		if (Slot.PlayerState == PlayerState)
		{
			if (LobbyStates.IsValidIndex(Index))
			{
				LobbyStates[Index] = NewState;
			}
			if (AvatarSlots.IsValidIndex(Index) && AvatarSlots[Index])
			{
				AvatarSlots[Index]->SetLobbyState(NewState);
			}
			return;
		}
	}
}

void ALobbyGameState::RefreshPlayerLobbyState(APlayerState* PlayerState)
{
	if (!PlayerState) return;
	for (int32 Index = 0; Index < LobbySlots.Num(); ++Index)
	{
		FPlayerLobbySlot& Slot = LobbySlots[Index];
		if (Slot.PlayerState == PlayerState)
		{
			if (LobbyStates.IsValidIndex(Index))
			{
				LobbyStates[Index] = Slot.bReady ? ELobbyAvatarState::Ready : ELobbyAvatarState::Wait;
			}
			if (AvatarSlots.IsValidIndex(Index) && AvatarSlots[Index])
			{
				AvatarSlots[Index]->SetLobbyState(LobbyStates.IsValidIndex(Index) ? LobbyStates[Index] : ELobbyAvatarState::Wait);
			}
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

ALobbyAvatarSlot* ALobbyGameState::GetAvatarSlot(int32 Index) const
{
	if (!AvatarSlots.IsValidIndex(Index)) return nullptr;
	return AvatarSlots[Index];
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

void ALobbyGameState::OnRep_LobbySlots()
{
	ApplyLobbySlotsToAvatars();
}

void ALobbyGameState::OnRep_LobbyStates()
{
	const int32 SlotCount = FMath::Min(LobbyStates.Num(), AvatarSlots.Num());
	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		if (AvatarSlots[Index])
		{
			AvatarSlots[Index]->SetLobbyState(LobbyStates[Index]);
		}
	}
}

void ALobbyGameState::ApplyLobbySlotsToAvatars()
{
	const int32 SlotCount = FMath::Min(LobbySlots.Num(), AvatarSlots.Num());
	if (LobbySlotPlayersCache.Num() != SlotCount)
	{
		LobbySlotPlayersCache.SetNum(SlotCount);
	}
	for (int32 Index = 0; Index < SlotCount; ++Index)
	{
		if (AvatarSlots[Index])
		{
			AMAPlayerState* PS = Cast<AMAPlayerState>(LobbySlots[Index].PlayerState.Get());
			const TWeakObjectPtr<APlayerState> PreviousPS = LobbySlotPlayersCache[Index];
			if (PreviousPS.Get() != PS)
			{
				AvatarSlots[Index]->SetOccupant(PS);
				LobbySlotPlayersCache[Index] = PS;
			}

			if (LobbyStates.IsValidIndex(Index))
			{
				AvatarSlots[Index]->SetLobbyState(LobbyStates[Index]);
			}
		}
	}
}

void ALobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyGameState, LobbySlots);
	DOREPLIFETIME(ALobbyGameState, LobbyStates);
}
