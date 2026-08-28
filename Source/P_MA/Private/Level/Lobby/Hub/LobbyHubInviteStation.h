#pragma once

#include "CoreMinimal.h"
#include "Level/Lobby/Hub/LobbyHubInteractableActor.h"
#include "LobbyHubInviteStation.generated.h"

class AMAPlayerCharacter;

/** World entry point for the platform invite flow. */
UCLASS()
class P_MA_API ALobbyHubInviteStation : public ALobbyHubInteractableActor
{
	GENERATED_BODY()

public:
	ALobbyHubInviteStation();

private:
	void HandleInteract(AMAPlayerCharacter* Interactor);
};
