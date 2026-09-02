#include "Player/MAPlayerControllerBase.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/MAGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/Camera/MACameraOcclusionCutoutComponent.h"
#include "Player/Camera/MAPlayerCameraDirectorComponent.h"
#include "Player/MAPlayerState.h"
#include "Widget/Settings/SettingsWidget.h"
#include "Widget/System/SystemMenuWidget.h"

AMAPlayerControllerBase::AMAPlayerControllerBase()
{
	CameraDirectorComponent = CreateDefaultSubobject<UMAPlayerCameraDirectorComponent>(TEXT("CameraDirectorComponent"));
	CameraOcclusionCutoutComponent =
		CreateDefaultSubobject<UMACameraOcclusionCutoutComponent>(TEXT("CameraOcclusionCutoutComponent"));
}

void AMAPlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (CameraDirectorComponent)
	{
		CameraDirectorComponent->RefreshPawnCamera();
	}
}

void AMAPlayerControllerBase::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);

	if (CameraDirectorComponent)
	{
		CameraDirectorComponent->RefreshPawnCamera();
	}
}

void AMAPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (SystemMenuInputMapping)
			{
				InputSubsystem->RemoveMappingContext(SystemMenuInputMapping);
				InputSubsystem->AddMappingContext(SystemMenuInputMapping, 1);
			}
		}
	}

	if (SystemMenuToggleInputAction)
	{
		if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EnhancedInputComp->BindAction(SystemMenuToggleInputAction, ETriggerEvent::Started, this, &AMAPlayerControllerBase::ToggleSystemMenu);
		}
	}
}

void AMAPlayerControllerBase::ServerNotifyLoaded_Implementation()
{
	if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
	{
		PS->SetLoadingComplete(true);
	}

	if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
	{
		GI->UpdateLoadingStatus();
	}
}

void AMAPlayerControllerBase::ToggleSystemMenu()
{
	if (!IsLocalController()) return;

	if (ActiveSettingsWidget && ActiveSettingsWidget->IsInViewport())
	{
		CloseSettingsWidget();
		return;
	}

	if (ActiveSystemMenuWidget && ActiveSystemMenuWidget->IsInViewport())
	{
		CloseSystemMenu();
		return;
	}

	if (!SystemMenuWidgetClass) return;

	ActiveSystemMenuWidget = CreateWidget<USystemMenuWidget>(this, SystemMenuWidgetClass);
	if (!ActiveSystemMenuWidget) return;

	ActiveSystemMenuWidget->OnActionRequested.RemoveAll(this);
	ActiveSystemMenuWidget->OnActionRequested.AddUObject(this, &AMAPlayerControllerBase::HandleSystemMenuActionRequested);
	ActiveSystemMenuWidget->AddToViewport(200);

	ApplySystemMenuOpenInputMode();
}

void AMAPlayerControllerBase::ApplySystemMenuOpenInputMode()
{
	ApplyWidgetFocusInputMode(ActiveSystemMenuWidget);
}

void AMAPlayerControllerBase::CloseSystemMenu()
{
	if (ActiveSystemMenuWidget)
	{
		ActiveSystemMenuWidget->RemoveFromParent();
		ActiveSystemMenuWidget = nullptr;
	}

	ApplySystemMenuClosedInputMode();
}

void AMAPlayerControllerBase::CloseSettingsWidget()
{
	if (!ActiveSettingsWidget) return;

	ActiveSettingsWidget->RemoveFromParent();
	ActiveSettingsWidget = nullptr;
	ApplyGameAndUiInputMode();
}

void AMAPlayerControllerBase::RefreshSettingsFocus()
{
	if (ActiveSettingsWidget && ActiveSettingsWidget->IsInViewport())
	{
		ApplyWidgetFocusInputMode(ActiveSettingsWidget);
		return;
	}

	ApplyGameAndUiInputMode();
}

void AMAPlayerControllerBase::ReopenSettingsWidget()
{
	if (!ActiveSettingsWidget || !ActiveSettingsWidget->IsInViewport()) return;

	const ESettingsCategory ActiveCategory = ActiveSettingsWidget->GetActiveCategory();
	CloseSettingsWidget();
	OpenSettingsWidget(ActiveCategory);
}

void AMAPlayerControllerBase::OpenSettings(ESettingsCategory InitialCategory)
{
	if (ActiveSystemMenuWidget && ActiveSystemMenuWidget->IsInViewport())
	{
		CloseSystemMenu();
	}

	OpenSettingsWidget(InitialCategory);
}

void AMAPlayerControllerBase::ApplySystemMenuClosedInputMode()
{
	ApplyGameAndUiInputMode();
}

void AMAPlayerControllerBase::ApplyWidgetFocusInputMode(UUserWidget* TargetWidget)
{
	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(TargetWidget->TakeWidget());
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AMAPlayerControllerBase::ApplyGameAndUiInputMode()
{
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AMAPlayerControllerBase::HandleSystemMenuActionRequested(ESystemMenuAction Action)
{
	switch (Action)
	{
	case ESystemMenuAction::Close:
		CloseSystemMenu();
		break;
	case ESystemMenuAction::Settings:
		CloseSystemMenu();
		OpenSettingsWidget(ESettingsCategory::Graphics);
		break;
	case ESystemMenuAction::Exit:
		UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
		break;
	default:
		break;
	}
}

void AMAPlayerControllerBase::OpenSettingsWidget(ESettingsCategory InitialCategory)
{
	if (ActiveSettingsWidget && ActiveSettingsWidget->IsInViewport())
	{
		ActiveSettingsWidget->SetActiveCategory(InitialCategory);
		ApplyWidgetFocusInputMode(ActiveSettingsWidget);
		return;
	}

	if (!SettingsWidgetClass) return;

	ActiveSettingsWidget = CreateWidget<USettingsWidget>(this, SettingsWidgetClass);
	if (!ActiveSettingsWidget) return;

	ActiveSettingsWidget->SetInitialCategory(InitialCategory);
	ActiveSettingsWidget->AddToViewport(210);
	ApplyWidgetFocusInputMode(ActiveSettingsWidget);
}
