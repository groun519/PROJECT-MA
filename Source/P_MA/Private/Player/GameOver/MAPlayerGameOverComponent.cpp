#include "Player/GameOver/MAPlayerGameOverComponent.h"

#include "Framework/MAGameMode.h"
#include "Framework/MAGameState.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/MAPlayerControllerBase.h"
#include "Player/Spectate/MAPlayerSpectateComponent.h"
#include "Widget/GameOver/MAGameOverWidget.h"
#include "Widget/Settings/SettingsCategoryButtonWidget.h"

UMAPlayerGameOverComponent::UMAPlayerGameOverComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMAPlayerGameOverComponent::BeginPlay()
{
	Super::BeginPlay();

	const APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	if (AMAGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr)
	{
		GameState->OnGameOverChanged.AddUObject(this, &UMAPlayerGameOverComponent::HandleGameOverChanged);
		HandleGameOverChanged(GameState->IsGameOver());
	}
}

void UMAPlayerGameOverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AMAGameState* GameState = GetWorld() ? GetWorld()->GetGameState<AMAGameState>() : nullptr)
	{
		GameState->OnGameOverChanged.RemoveAll(this);
	}

	RemoveGameOverWidget();
	Super::EndPlay(EndPlayReason);
}

void UMAPlayerGameOverComponent::HandleGameOverChanged(bool bGameOver)
{
	if (!bGameOver)
	{
		RemoveGameOverWidget();
		return;
	}

	if (UMAPlayerSpectateComponent* SpectateComponent = GetOwner()->FindComponentByClass<UMAPlayerSpectateComponent>())
	{
		SpectateComponent->StopSpectating();
	}

	ShowGameOverWidget();
}

void UMAPlayerGameOverComponent::ShowGameOverWidget()
{
	AMAPlayerControllerBase* PlayerController = Cast<AMAPlayerControllerBase>(GetOwner());
	if (!PlayerController || !PlayerController->IsLocalController() || !GameOverWidgetClass) return;

	if (!GameOverWidget)
	{
		GameOverWidget = CreateWidget<UMAGameOverWidget>(PlayerController, GameOverWidgetClass);
		if (!GameOverWidget) return;

		GameOverWidget->OnActionRequested.AddUObject(this, &UMAPlayerGameOverComponent::HandleGameOverActionRequested);
		GameOverWidget->AddToViewport(150);
	}

	GameOverWidget->SetHostControls(PlayerController->HasAuthority());
	PlayerController->ApplyWidgetFocusInputMode(GameOverWidget);
}

void UMAPlayerGameOverComponent::RemoveGameOverWidget()
{
	if (!GameOverWidget) return;

	GameOverWidget->RemoveFromParent();
	GameOverWidget = nullptr;

	if (AMAPlayerControllerBase* PlayerController = Cast<AMAPlayerControllerBase>(GetOwner()))
	{
		PlayerController->ApplyGameAndUiInputMode();
	}
}

void UMAPlayerGameOverComponent::HandleGameOverActionRequested(EMAGameOverAction Action)
{
	AMAPlayerControllerBase* PlayerController = Cast<AMAPlayerControllerBase>(GetOwner());
	if (!PlayerController) return;

	switch (Action)
	{
	case EMAGameOverAction::ReturnToLobby:
		ServerRequestReturnToLobby();
		break;
	case EMAGameOverAction::Settings:
		PlayerController->OpenSettings(ESettingsCategory::Graphics);
		break;
	case EMAGameOverAction::Exit:
		UKismetSystemLibrary::QuitGame(PlayerController, PlayerController, EQuitPreference::Quit, false);
		break;
	default:
		break;
	}
}

void UMAPlayerGameOverComponent::ServerRequestReturnToLobby_Implementation()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController) return;

	if (AMAGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMAGameMode>() : nullptr)
	{
		GameMode->RequestReturnToLobby(PlayerController);
	}
}
