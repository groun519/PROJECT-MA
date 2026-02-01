// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "LobbyGameState.h"
#include "Player/MAPlayerState.h"
#include "GameFramework/GameStateBase.h"

ALobbyGameMode::ALobbyGameMode()
{
	GameStateClass = ALobbyGameState::StaticClass();
	PlayerStateClass = AMAPlayerState::StaticClass();
	DefaultPawnClass = nullptr;
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ALobbyGameState* LGS = GetGameState<ALobbyGameState>())
	{
		LGS->OnSlotsRegistered.AddUObject(this, &ALobbyGameMode::HandleSlotsRegistered);
	}
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ALobbyGameState* LGS = GetGameState<ALobbyGameState>())
	{
		if (AMAPlayerState* LPS = NewPlayer ? NewPlayer->GetPlayerState<AMAPlayerState>() : nullptr)
		{
			LGS->AssignSlotToPlayer(LPS);
			if (NewPlayer && NewPlayer->IsLocalController())
			{
				LGS->SetPlayerReady(LPS, true);
			}
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

void ALobbyGameMode::HandleSlotsRegistered()
{
	if (ALobbyGameState* LGS = GetGameState<ALobbyGameState>())
	{
		for (APlayerState* PS : LGS->PlayerArray)
		{
			AMAPlayerState* MAState = Cast<AMAPlayerState>(PS);
			if (!MAState)
			{
				continue;
			}
			if (LGS->GetSlotIndex(MAState) == INDEX_NONE)
			{
				LGS->AssignSlotToPlayer(MAState);
			}
		}
	}
}
