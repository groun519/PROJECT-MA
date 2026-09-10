#include "Level/Lobby/Hub/LobbyHubGameMode.h"

#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Level/Lobby/Hub/LobbyHubArrivalVolume.h"
#include "Level/Lobby/Hub/LobbyHubCharacter.h"
#include "Level/Lobby/Hub/LobbyHubPlayerController.h"
#include "Player/MAPlayerState.h"

ALobbyHubGameMode::ALobbyHubGameMode()
{
	GameStateClass = AGameStateBase::StaticClass();
	PlayerStateClass = AMAPlayerState::StaticClass();
	PlayerControllerClass = ALobbyHubPlayerController::StaticClass();
	DefaultPawnClass = ALobbyHubCharacter::StaticClass();
}

void ALobbyHubGameMode::RestartPlayer(AController* NewPlayer)
{
	if (ALobbyHubArrivalVolume* ArrivalVolume = FindArrivalVolume())
	{
		FLobbyHubArrivalSpawn ArrivalSpawn;
		if (ArrivalVolume->TryCreateArrivalSpawn(ArrivalSpawn))
		{
			RestartPlayerAtTransform(NewPlayer, ArrivalSpawn.SpawnTransform);
			if (ALobbyHubCharacter* HubCharacter = Cast<ALobbyHubCharacter>(NewPlayer->GetPawn()))
			{
				ArrivalVolume->Launch(*HubCharacter, ArrivalSpawn);
			}
			return;
		}
	}

	Super::RestartPlayer(NewPlayer);
}

ALobbyHubArrivalVolume* ALobbyHubGameMode::FindArrivalVolume() const
{
	ALobbyHubArrivalVolume* FoundVolume = nullptr;
	for (TActorIterator<ALobbyHubArrivalVolume> It(GetWorld()); It; ++It)
	{
		if (!ensureMsgf(!FoundVolume, TEXT("Lobby Hub requires exactly one Arrival Volume.")))
		{
			return nullptr;
		}

		FoundVolume = *It;
	}

	ensureMsgf(FoundVolume, TEXT("Lobby Hub Arrival Volume is missing. Falling back to PlayerStart."));
	return FoundVolume;
}
