#include "LobbyHubPlayerController.h"

#include "Audio/Music/MAMusicSubsystem.h"
#include "Framework/MAGameInstance.h"
#include "Level/Lobby/Hub/LobbyHubCharacter.h"
#include "Player/MAPlayerState.h"

void ALobbyHubPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	// TODO(LevelSystem): The active space must own this MusicTag once space activation exists.
	// This controller is only the local-entry bridge for the current Hub-only phase.
	if (UMAMusicSubsystem* MusicSubsystem = GetGameInstance()->GetSubsystem<UMAMusicSubsystem>())
	{
		MusicSubsystem->PlayMusic(FGameplayTag::RequestGameplayTag(TEXT("Music.Lobby")));
	}

	UMAGameInstance* GameInstance = GetGameInstance<UMAGameInstance>();
	if (!GameInstance) return;

	FLoadoutSelection SavedLoadout;
	if (!GameInstance->LoadLoadout(SavedLoadout)) return;

	if (HasAuthority())
	{
		ApplySavedLoadout(SavedLoadout);
	}
	else
	{
		ServerApplySavedLoadout(SavedLoadout);
	}
}

void ALobbyHubPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	if (ALobbyHubCharacter* HubCharacter = Cast<ALobbyHubCharacter>(NewPawn))
	{
		HubCharacter->InitializeHubRuntime();
	}
}

void ALobbyHubPlayerController::AcknowledgePossession(APawn* NewPawn)
{
	Super::AcknowledgePossession(NewPawn);

	if (ALobbyHubCharacter* HubCharacter = Cast<ALobbyHubCharacter>(NewPawn))
	{
		HubCharacter->InitializeHubRuntime();
	}
}

void ALobbyHubPlayerController::ApplySavedLoadout(const FLoadoutSelection& Loadout)
{
	if (AMAPlayerState* MAPlayerState = GetPlayerState<AMAPlayerState>())
	{
		MAPlayerState->SetLoadoutSelection(Loadout);
	}
}

void ALobbyHubPlayerController::ServerApplySavedLoadout_Implementation(const FLoadoutSelection& Loadout)
{
	ApplySavedLoadout(Loadout);
}
