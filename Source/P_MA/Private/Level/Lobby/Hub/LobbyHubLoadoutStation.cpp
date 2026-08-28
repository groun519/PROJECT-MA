#include "Level/Lobby/Hub/LobbyHubLoadoutStation.h"

#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Convenience/MAInteractableComponent.h"
#include "Framework/MAGameInstance.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Level/Lobby/Hub/LobbyHubPlayerController.h"
#include "Player/Camera/MAPlayerCameraDirectorComponent.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerState.h"
#include "Widget/Lobby/LobbyReadyStartWidget.h"
#include "Widget/Lobby/LobbyWidgetRoot.h"
#include "Widget/Lobby/Loadout/LoadoutWidget.h"

ALobbyHubLoadoutStation::ALobbyHubLoadoutStation()
{
	InteractableComponent->CALL_SETUP_INTERACT(HandleInteract);

	LoadoutCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("LoadoutCameraComponent"));
	LoadoutCameraComponent->SetupAttachment(RootComponent);
	LoadoutCameraComponent->SetFieldOfView(CameraFov);
}

void ALobbyHubLoadoutStation::HandleInteract(AMAPlayerCharacter* Interactor)
{
	if (!Interactor || !Interactor->IsLocallyControlled()) return;
	OpenLoadoutFor(*Interactor);
}

void ALobbyHubLoadoutStation::OpenLoadoutFor(AMAPlayerCharacter& Interactor)
{
	if (!LoadoutRootWidgetClass || (ActiveLoadoutWidget && ActiveLoadoutWidget->IsInViewport())) return;

	ALobbyHubPlayerController* HubController =
		Cast<ALobbyHubPlayerController>(Interactor.GetController());
	if (!HubController) return;

	const AMAPlayerState* PlayerState = HubController->GetPlayerState<AMAPlayerState>();
	if (!PlayerState) return;

	PendingLoadout = PlayerState->GetLoadoutSelection();
	ActivePlayerController = HubController;
	ActiveInteractor = &Interactor;

	ActiveLoadoutWidget = CreateWidget<ULobbyWidgetRoot>(HubController, LoadoutRootWidgetClass);
	if (!ActiveLoadoutWidget)
	{
		ActivePlayerController.Reset();
		ActiveInteractor.Reset();
		return;
	}
	ActiveLoadoutWidget->AddToViewport(100);

	if (ActiveLoadoutWidget->LobbyReadyStartWidget)
	{
		ActiveLoadoutWidget->LobbyReadyStartWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ActiveLoadoutWidget->LoadoutButton)
	{
		ActiveLoadoutWidget->LoadoutButton->OnClicked.AddUniqueDynamic(
			this,
			&ALobbyHubLoadoutStation::CloseLoadout);
	}
	if (ActiveLoadoutWidget->LoadoutButtonText)
	{
		ActiveLoadoutWidget->LoadoutButtonText->SetText(
			NSLOCTEXT("LobbyHub", "SaveLoadout", "Save"));
	}
	if (ActiveLoadoutWidget->LoadoutWidget)
	{
		BindLoadoutWidget(*ActiveLoadoutWidget->LoadoutWidget);
		ActiveLoadoutWidget->LoadoutWidget->ActivateBodyTabUI();
		ActiveLoadoutWidget->LoadoutWidget->SyncSelectionFromPending(PendingLoadout);
		ActiveLoadoutWidget->LoadoutWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	UpdateLoadoutCamera(Interactor);
	EnterLoadoutPresentation(*HubController);
	HubController->ApplyWidgetFocusInputMode(ActiveLoadoutWidget);
}

void ALobbyHubLoadoutStation::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ActiveLoadoutWidget)
	{
		ActiveLoadoutWidget->RemoveFromParent();
		ActiveLoadoutWidget = nullptr;
	}
	if (ALobbyHubPlayerController* HubController = ActivePlayerController.Get())
	{
		ExitLoadoutPresentation(*HubController);
		HubController->ApplyGameAndUiInputMode();
	}
	else
	{
		SetInteractorRotationLocked(false);
	}
	ActivePlayerController.Reset();
	ActiveInteractor.Reset();

	Super::EndPlay(EndPlayReason);
}

void ALobbyHubLoadoutStation::BindLoadoutWidget(ULoadoutWidget& LoadoutWidget)
{
	LoadoutWidget.OnBodyColorSelected().RemoveAll(this);
	LoadoutWidget.OnBodyColorSelected().AddUObject(this, &ALobbyHubLoadoutStation::HandleBodyColorSelected);
	LoadoutWidget.OnEyeColorSelected().RemoveAll(this);
	LoadoutWidget.OnEyeColorSelected().AddUObject(this, &ALobbyHubLoadoutStation::HandleEyeColorSelected);
	LoadoutWidget.OnEyeShapeSelected().RemoveAll(this);
	LoadoutWidget.OnEyeShapeSelected().AddUObject(this, &ALobbyHubLoadoutStation::HandleEyeShapeSelected);
	LoadoutWidget.OnWeaponSelected().RemoveAll(this);
	LoadoutWidget.OnWeaponSelected().AddUObject(this, &ALobbyHubLoadoutStation::HandleWeaponSelected);
	LoadoutWidget.OnMountSelected().RemoveAll(this);
	LoadoutWidget.OnMountSelected().AddUObject(this, &ALobbyHubLoadoutStation::HandleMountSelected);
}

void ALobbyHubLoadoutStation::UpdateLoadoutCamera(const AMAPlayerCharacter& Interactor)
{
	const FVector FacingDirection = Interactor.GetActorForwardVector().GetSafeNormal2D();
	const FVector TargetLocation = Interactor.GetActorLocation()
		+ FVector::UpVector * CameraTargetHeight;
	const FVector CameraLocation = Interactor.GetActorLocation()
		+ FacingDirection * CameraDistance
		+ FVector::UpVector * CameraHeight;

	LoadoutCameraComponent->SetWorldLocationAndRotation(
		CameraLocation,
		(TargetLocation - CameraLocation).Rotation());
	LoadoutCameraComponent->SetFieldOfView(CameraFov);
}

void ALobbyHubLoadoutStation::EnterLoadoutPresentation(ALobbyHubPlayerController& PlayerController)
{
	PlayerController.SetIgnoreMoveInput(true);
	PlayerController.SetIgnoreLookInput(true);
	SetInteractorRotationLocked(true);

	if (UMAPlayerCameraDirectorComponent* CameraDirector = PlayerController.GetCameraDirector())
	{
		CameraDirector->SwitchToViewTarget(this, CameraBlendTime);
	}
}

void ALobbyHubLoadoutStation::ExitLoadoutPresentation(ALobbyHubPlayerController& PlayerController)
{
	if (UMAPlayerCameraDirectorComponent* CameraDirector = PlayerController.GetCameraDirector())
	{
		CameraDirector->SwitchToPawnCamera(CameraBlendTime);
	}

	SetInteractorRotationLocked(false);
	PlayerController.SetIgnoreMoveInput(false);
	PlayerController.SetIgnoreLookInput(false);
}

void ALobbyHubLoadoutStation::SetInteractorRotationLocked(const bool bLocked)
{
	if (bRotationLocked == bLocked) return;

	AMAPlayerCharacter* Interactor = ActiveInteractor.Get();
	UAbilitySystemComponent* AbilitySystem = Interactor
		? Interactor->GetAbilitySystemComponent()
		: nullptr;
	if (!AbilitySystem)
	{
		if (!bLocked) bRotationLocked = false;
		return;
	}

	const FGameplayTag RotationLockTag = UMAAbilitySystemStatics::GetRotationLockTag();
	if (bLocked)
	{
		AbilitySystem->AddLooseGameplayTag(RotationLockTag);
	}
	else
	{
		AbilitySystem->RemoveLooseGameplayTag(RotationLockTag);
	}
	bRotationLocked = bLocked;
}

void ALobbyHubLoadoutStation::ApplyPendingLoadout()
{
	if (ALobbyHubPlayerController* HubController = ActivePlayerController.Get())
	{
		HubController->SetLoadoutSelection(PendingLoadout);
	}
}

void ALobbyHubLoadoutStation::HandleBodyColorSelected(const FMaterialParamData& BodyData)
{
	PendingLoadout.Color.BodyData = BodyData;
	ApplyPendingLoadout();
}

void ALobbyHubLoadoutStation::HandleEyeColorSelected(const FMaterialParamData& EyeData)
{
	PendingLoadout.Color.EyeData = EyeData;
	ApplyPendingLoadout();
}

void ALobbyHubLoadoutStation::HandleEyeShapeSelected(const FName EyeShapeId)
{
	PendingLoadout.EyeShapeId = EyeShapeId;
	ApplyPendingLoadout();
}

void ALobbyHubLoadoutStation::HandleWeaponSelected(
	const FName WeaponId,
	USkeletalMesh*,
	const FTransform&)
{
	PendingLoadout.WeaponId = WeaponId;
	ApplyPendingLoadout();
}

void ALobbyHubLoadoutStation::HandleMountSelected(const FName MountId)
{
	PendingLoadout.MountId = MountId;
	ApplyPendingLoadout();
}

void ALobbyHubLoadoutStation::CloseLoadout()
{
	ALobbyHubPlayerController* HubController = ActivePlayerController.Get();
	if (!ActiveLoadoutWidget) return;

	if (HubController)
	{
		if (UMAGameInstance* GameInstance = HubController->GetGameInstance<UMAGameInstance>())
		{
			GameInstance->SaveLoadout(PendingLoadout);
		}

		ExitLoadoutPresentation(*HubController);
		HubController->ApplyGameAndUiInputMode();
	}
	else
	{
		SetInteractorRotationLocked(false);
	}

	ActiveLoadoutWidget->RemoveFromParent();
	ActiveLoadoutWidget = nullptr;
	ActivePlayerController.Reset();
	ActiveInteractor.Reset();
}
