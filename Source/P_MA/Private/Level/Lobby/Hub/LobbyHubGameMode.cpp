#include "Level/Lobby/Hub/LobbyHubGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "Level/Lobby/Hub/LobbyHubCharacter.h"
#include "Level/Lobby/Hub/LobbyHubPlayerController.h"
#include "Player/MAPlayerState.h"

ALobbyHubGameMode::ALobbyHubGameMode()
{
	GameStateClass = AGameStateBase::StaticClass();
	PlayerStateClass = AMAPlayerState::StaticClass();
	PlayerControllerClass = ALobbyHubPlayerController::StaticClass();
	DefaultPawnClass = ALobbyHubCharacter::StaticClass();
	bUseSeamlessTravel = true;
}
