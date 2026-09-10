#include "Player/MAPlayerControllerBase.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/MAGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Level/Transition/MASpaceTransitionSubsystem.h"
#include "Player/Camera/MACameraLibrary.h"
#include "Player/Camera/MACameraOcclusionCutoutComponent.h"
#include "Player/MAPlayerState.h"
#include "TimerManager.h"
#include "Widget/Settings/SettingsWidget.h"
#include "Widget/System/SystemMenuWidget.h"

AMAPlayerControllerBase::AMAPlayerControllerBase()
{
	CameraOcclusionCutoutComponent =
		CreateDefaultSubobject<UMACameraOcclusionCutoutComponent>(TEXT("CameraOcclusionCutoutComponent"));
}

void AMAPlayerControllerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(CameraFadeTimerHandle);
	FMACameraLibrary::StopFade(*this);
	Super::EndPlay(EndPlayReason);
}

void AMAPlayerControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RefreshCameraOcclusion(InPawn);
}

void AMAPlayerControllerBase::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	RefreshCameraOcclusion(P);
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

void AMAPlayerControllerBase::ClientPrepareSpaceTransition_Implementation(
	const FMASpaceTransitionRequest& Request)
{
	if (UMASpaceTransitionSubsystem* SpaceTransition =
		GetWorld()->GetSubsystem<UMASpaceTransitionSubsystem>())
	{
		SpaceTransition->BeginClientPrepare(Request);
	}
}

void AMAPlayerControllerBase::ClientCloseSpaceTransition_Implementation()
{
	if (UMASpaceTransitionSubsystem* SpaceTransition =
		GetWorld()->GetSubsystem<UMASpaceTransitionSubsystem>())
	{
		SpaceTransition->BeginLocalClose();
	}
}

void AMAPlayerControllerBase::ClientOpenSpaceTransition_Implementation()
{
	if (UMASpaceTransitionSubsystem* SpaceTransition =
		GetWorld()->GetSubsystem<UMASpaceTransitionSubsystem>())
	{
		SpaceTransition->BeginLocalOpen();
	}
}

void AMAPlayerControllerBase::ClientAbortSpaceTransition_Implementation()
{
	if (UMASpaceTransitionSubsystem* SpaceTransition =
		GetWorld()->GetSubsystem<UMASpaceTransitionSubsystem>())
	{
		SpaceTransition->AbortLocalTransition();
	}
}

void AMAPlayerControllerBase::ServerNotifySpaceTransitionProgress_Implementation(
	const FString& DestinationInstanceIdentity,
	const bool bSucceeded)
{
	if (UMASpaceTransitionSubsystem* SpaceTransition =
		GetWorld()->GetSubsystem<UMASpaceTransitionSubsystem>())
	{
		SpaceTransition->HandleClientProgress(*this, DestinationInstanceIdentity, bSucceeded);
	}
}

void AMAPlayerControllerBase::RequestCameraFade(const FMACameraFadeSettings& Settings)
{
	if (IsLocalController())
	{
		PlayCameraFade(Settings);
	}
	else if (HasAuthority())
	{
		ClientPlayCameraFade(Settings);
	}
}

void AMAPlayerControllerBase::ClientPlayCameraFade_Implementation(const FMACameraFadeSettings& Settings)
{
	PlayCameraFade(Settings);
}

void AMAPlayerControllerBase::PlayCameraFade(const FMACameraFadeSettings& Settings)
{
	GetWorldTimerManager().ClearTimer(CameraFadeTimerHandle);
	FMACameraLibrary::StopFade(*this);

	const float FadeOutSeconds = FMath::Max(0.f, Settings.FadeOutSeconds);
	const float FadeInSeconds = FMath::Max(0.f, Settings.FadeInSeconds);
	if (FadeOutSeconds <= 0.f)
	{
		if (FadeInSeconds > 0.f) FMACameraLibrary::FadeIn(*this, FadeInSeconds);
		return;
	}

	FMACameraLibrary::FadeOut(*this, FadeOutSeconds);
	GetWorldTimerManager().SetTimer(
		CameraFadeTimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, FadeInSeconds]()
		{
			if (FadeInSeconds > 0.f)
			{
				FMACameraLibrary::FadeIn(*this, FadeInSeconds);
			}
			else
			{
				FMACameraLibrary::StopFade(*this);
			}
		}),
		FadeOutSeconds,
		false);
}

void AMAPlayerControllerBase::RefreshCameraOcclusion(APawn* InPawn)
{
	if (!IsLocalController() || !CameraOcclusionCutoutComponent) return;

	CameraOcclusionCutoutComponent->ClearTarget();
	if (InPawn)
	{
		CameraOcclusionCutoutComponent->RevealTarget(*this, *InPawn);
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
