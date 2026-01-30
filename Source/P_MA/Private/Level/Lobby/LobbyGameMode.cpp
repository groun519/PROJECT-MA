// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "Player/MAPlayerState.h"

ALobbyGameMode::ALobbyGameMode()
{
	GameStateClass = ALobbyGameState::StaticClass();
	PlayerStateClass = AMAPlayerState::StaticClass();
	DefaultPawnClass = nullptr;
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ALobbyGameState* LGS = GetGameState<ALobbyGameState>())
	{
		if (AMAPlayerState* LPS = NewPlayer ? NewPlayer->GetPlayerState<AMAPlayerState>() : nullptr)
		{
			LGS->AssignSlotToPlayer(LPS);
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	if (ALobbyGameState* LGS = GetGameState<ALobbyGameState>())
	{
		if (AMAPlayerState* LPS = Exiting ? Exiting->GetPlayerState<AMAPlayerState>() : nullptr)
		{
			LGS->RemovePlayerFromSlot(LPS);
		}
	}

	Super::Logout(Exiting);
}
