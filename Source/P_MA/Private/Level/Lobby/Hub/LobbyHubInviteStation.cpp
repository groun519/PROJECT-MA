#include "Level/Lobby/Hub/LobbyHubInviteStation.h"

#include "Convenience/MAInteractableComponent.h"
#include "Framework/MAGameInstance.h"
#include "Player/MAPlayerCharacter.h"

ALobbyHubInviteStation::ALobbyHubInviteStation()
{
	InteractableComponent->CALL_SETUP_INTERACT(HandleInteract);
}

void ALobbyHubInviteStation::HandleInteract(AMAPlayerCharacter* Interactor)
{
	if (!Interactor || !Interactor->IsLocallyControlled()) return;

	if (UMAGameInstance* GameInstance = Interactor->GetGameInstance<UMAGameInstance>())
	{
		GameInstance->ShowInviteUI();
	}
}
