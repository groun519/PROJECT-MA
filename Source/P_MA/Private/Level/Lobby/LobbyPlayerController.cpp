// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyPlayerController.h"
#include "LobbyGameState.h"
#include "LobbyAvatarSlot.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Widget/Lobby/LobbyWidgetRoot.h"
#include "Widget/Lobby/LobbyReadyStartWidget.h"
#include "Widget/Lobby/LoadoutWidget.h"
#include "Framework/MAGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Player/MAPlayerState.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocalController())
	{
		ShowLobbyUI();

		bShowMouseCursor = true;
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);

		if (LobbyRootWidgetClass)
		{
			LobbyRootWidgetInstance = CreateWidget<ULobbyWidgetRoot>(this, LobbyRootWidgetClass);
			if (LobbyRootWidgetInstance)
			{
				LobbyRootWidgetInstance->AddToViewport();
				if (LobbyRootWidgetInstance->LobbyReadyStartWidget &&
					LobbyRootWidgetInstance->LobbyReadyStartWidget->ReadyStartButton)
				{
					LobbyRootWidgetInstance->LobbyReadyStartWidget->ReadyStartButton->OnClicked.AddDynamic(
						this,
						&ALobbyPlayerController::HandleReadyStartClicked
					);
				}

				if (LobbyRootWidgetInstance->LoadoutButton)
				{
					LobbyRootWidgetInstance->LoadoutButton->OnClicked.AddDynamic(
						this,
						&ALobbyPlayerController::HandleLoadoutClicked
					);
				}

				if (LobbyRootWidgetInstance->LoadoutButtonText)
				{
					LobbyRootWidgetInstance->LoadoutButtonText->SetText(FText::FromString(TEXT("Loadout")));
				}

				if (LobbyRootWidgetInstance->LoadoutWidget)
				{
					LobbyRootWidgetInstance->LoadoutWidget->SetVisibility(ESlateVisibility::Collapsed);
				}
			}
		}

		TArray<AActor*> TaggedActors;
		UGameplayStatics::GetAllActorsWithTag(this, LobbyCameraTag, TaggedActors);
		if (TaggedActors.Num() > 0 && TaggedActors[0])
		{
			LobbyCameraActor = TaggedActors[0];
			SetViewTargetWithBlend(LobbyCameraActor, 0.0f);
			LobbyCameraTransform = LobbyCameraActor->GetActorTransform();
			TargetCameraTransform = LobbyCameraTransform;

			LobbyCameraComponent = LobbyCameraActor->FindComponentByClass<UCameraComponent>();
			if (LobbyCameraComponent)
			{
				if (LobbyFov > 0.0f)
				{
					LobbyCameraComponent->SetFieldOfView(LobbyFov);
				}
				TargetFov = LobbyCameraComponent->FieldOfView;
			}
		}
	}

	GetWorldTimerManager().SetTimer(
		LobbyUiTimerHandle,
		this,
		&ALobbyPlayerController::UpdateLobbyUI,
		0.2f,
		true
	);
}

void ALobbyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LobbyUiTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ALobbyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocalController() || !LobbyCameraActor)
	{
		return;
	}

	if (CameraInterpSpeed > 0.0f)
	{
		const FVector CurLocation = LobbyCameraActor->GetActorLocation();
		const FRotator CurRotation = LobbyCameraActor->GetActorRotation();
		const FVector NewLocation = FMath::VInterpTo(
			CurLocation,
			TargetCameraTransform.GetLocation(),
			DeltaTime,
			CameraInterpSpeed
		);
		const FRotator NewRotation = FMath::RInterpTo(
			CurRotation,
			TargetCameraTransform.Rotator(),
			DeltaTime,
			CameraInterpSpeed
		);
		LobbyCameraActor->SetActorLocationAndRotation(NewLocation, NewRotation);
	}

	if (LobbyCameraComponent && FovInterpSpeed > 0.0f)
	{
		const float NewFov = FMath::FInterpTo(
			LobbyCameraComponent->FieldOfView,
			TargetFov,
			DeltaTime,
			FovInterpSpeed
		);
		LobbyCameraComponent->SetFieldOfView(NewFov);
	}
}

void ALobbyPlayerController::SetReady(bool bNewReady)
{
	ServerSetReady(bNewReady);
}

void ALobbyPlayerController::ServerSetReady_Implementation(bool bNewReady)
{
	if (ALobbyGameState* LGS = GetWorld() ? GetWorld()->GetGameState<ALobbyGameState>() : nullptr)
	{
		LGS->SetPlayerReady(GetPlayerState<APlayerState>(), bNewReady);
	}
}

void ALobbyPlayerController::ShowInviteUI()
{
	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface();
		if (ExternalUI.IsValid())
		{
			ExternalUI->ShowInviteUI(0);
		}
	}
}

void ALobbyPlayerController::PreviewEyeColor(const FLinearColor& EyeColor)
{
	if (!bHasPendingLoadoutColor)
	{
		if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
		{
			PendingLoadoutColor = PS->GetLoadoutColor();
		}
		bHasPendingLoadoutColor = true;
	}

	PendingLoadoutColor.EyeData.Color = EyeColor;
	ApplyPreviewColor(PendingLoadoutColor);
}

void ALobbyPlayerController::HandleReadyStartClicked()
{
	const bool bIsHost = HasAuthority() && IsLocalController();
	if (bIsHost)
	{
		if (ULobbyWidgetRoot* Root = LobbyRootWidgetInstance)
		{
			if (ALobbyGameState* LGS = GetWorld() ? GetWorld()->GetGameState<ALobbyGameState>() : nullptr)
			{
				const int32 ReadyCount = LGS->GetReadyCount();
				const int32 TotalCount = LGS->GetPlayerCount();
				if (TotalCount > 0 && ReadyCount == TotalCount)
				{
					if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
					{
						GI->StartSession();
					}
					if (UWorld* World = GetWorld())
					{
						World->ServerTravel(TEXT("/Game/Map/MainMap?listen"));
					}
				}
			}
		}
		return;
	}

	if (ALobbyGameState* LGS = GetWorld() ? GetWorld()->GetGameState<ALobbyGameState>() : nullptr)
	{
		const bool bIsReady = LGS->IsPlayerReady(GetPlayerState<APlayerState>());
		SetReady(!bIsReady);
	}
}

void ALobbyPlayerController::HandleLoadoutClicked()
{
	if (!IsLocalController())
	{
		return;
	}

	if (bInLoadoutView)
	{
		ExitLoadoutView();
	}
	else
	{
		EnterLoadoutView();
	}
}

void ALobbyPlayerController::UpdateLobbyUI()
{
	if (!LobbyRootWidgetInstance || !LobbyRootWidgetInstance->LobbyReadyStartWidget) return;

	if (ALobbyGameState* LGS = GetWorld() ? GetWorld()->GetGameState<ALobbyGameState>() : nullptr)
	{
		const int32 ReadyCount = LGS->GetReadyCount();
		const int32 TotalCount = LGS->GetPlayerCount();
		const bool bIsHost = HasAuthority() && IsLocalController();
		const bool bIsReady = bIsHost ? true : LGS->IsPlayerReady(GetPlayerState<APlayerState>());

		LobbyRootWidgetInstance->LobbyReadyStartWidget->UpdateStatus(
			bIsHost,
			bIsReady,
			ReadyCount,
			TotalCount
		);

		if (LobbyRootWidgetInstance->LobbyReadyStartWidget->ReadyStartButton)
		{
			const bool bEnableStart = bIsHost && (TotalCount > 0 && ReadyCount == TotalCount);
			LobbyRootWidgetInstance->LobbyReadyStartWidget->ReadyStartButton->SetIsEnabled(
				bIsHost ? bEnableStart : true
			);
		}
	}
}

void ALobbyPlayerController::EnterLoadoutView()
{
	bInLoadoutView = true;
	UpdateCameraTarget();

	if (LoadoutFov > 0.0f)
	{
		TargetFov = LoadoutFov;
	}

	if (ALobbyGameState* LGS = GetWorld() ? GetWorld()->GetGameState<ALobbyGameState>() : nullptr)
	{
		const int32 MySlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
		const TArray<TObjectPtr<ALobbyAvatarSlot>>& Slots = LGS->GetAvatarSlots();
		for (int32 Index = 0; Index < Slots.Num(); ++Index)
		{
			if (Slots[Index])
			{
				Slots[Index]->SetLocalHidden(Index != MySlotIndex);
			}
		}
	}

	if (LobbyRootWidgetInstance && LobbyRootWidgetInstance->LoadoutButtonText)
	{
		LobbyRootWidgetInstance->LoadoutButtonText->SetText(FText::FromString(TEXT("Save")));
	}

	if (!bHasPendingLoadoutColor)
	{
		if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
		{
			PendingLoadoutColor = PS->GetLoadoutColor();
			bHasPendingLoadoutColor = true;
		}
	}

	if (LobbyRootWidgetInstance && LobbyRootWidgetInstance->LoadoutWidget)
	{
		LobbyRootWidgetInstance->LoadoutWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void ALobbyPlayerController::ExitLoadoutView()
{
	CommitLoadoutColor();
	bInLoadoutView = false;
	UpdateCameraTarget();

	if (LobbyFov > 0.0f)
	{
		TargetFov = LobbyFov;
	}

	if (ALobbyGameState* LGS = GetWorld() ? GetWorld()->GetGameState<ALobbyGameState>() : nullptr)
	{
		const TArray<TObjectPtr<ALobbyAvatarSlot>>& Slots = LGS->GetAvatarSlots();
		for (ALobbyAvatarSlot* Slot : Slots)
		{
			if (Slot)
			{
				Slot->SetLocalHidden(false);
			}
		}
	}

	if (LobbyRootWidgetInstance && LobbyRootWidgetInstance->LoadoutButtonText)
	{
		LobbyRootWidgetInstance->LoadoutButtonText->SetText(FText::FromString(TEXT("Loadout")));
	}

	if (LobbyRootWidgetInstance && LobbyRootWidgetInstance->LoadoutWidget)
	{
		LobbyRootWidgetInstance->LoadoutWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ALobbyPlayerController::UpdateCameraTarget()
{
	if (!LobbyCameraActor)
	{
		return;
	}

	if (!bInLoadoutView)
	{
		TargetCameraTransform = LobbyCameraTransform;
		return;
	}

	ALobbyGameState* LGS = GetWorld() ? GetWorld()->GetGameState<ALobbyGameState>() : nullptr;
	if (!LGS)
	{
		TargetCameraTransform = LobbyCameraTransform;
		return;
	}

	const int32 SlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
	ALobbyAvatarSlot* Slot = LGS->GetAvatarSlot(SlotIndex);
	if (!Slot)
	{
		TargetCameraTransform = LobbyCameraTransform;
		return;
	}

	const FTransform SlotTransform = Slot->GetActorTransform();
	const FVector WorldLocation = SlotTransform.TransformPosition(LoadoutCameraOffset.GetLocation());
	const FQuat WorldRotation = SlotTransform.GetRotation() * LoadoutCameraOffset.GetRotation();
	TargetCameraTransform = FTransform(WorldRotation, WorldLocation, FVector::OneVector);
}

void ALobbyPlayerController::ApplyPreviewColor(const FMaterialParamDataPair& ColorData)
{
	if (ALobbyGameState* LGS = GetWorld() ? GetWorld()->GetGameState<ALobbyGameState>() : nullptr)
	{
		const int32 SlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
		if (ALobbyAvatarSlot* Slot = LGS->GetAvatarSlot(SlotIndex))
		{
			Slot->ApplyLoadoutColor(ColorData);
		}
	}
}

void ALobbyPlayerController::CommitLoadoutColor()
{
	if (!bHasPendingLoadoutColor)
	{
		return;
	}

	if (HasAuthority())
	{
		if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
		{
			PS->SetLoadoutColor(PendingLoadoutColor);
		}
	}
	else
	{
		ServerSetLoadoutColor(PendingLoadoutColor);
	}
}

void ALobbyPlayerController::ServerSetLoadoutColor_Implementation(const FMaterialParamDataPair& ColorData)
{
	if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
	{
		PS->SetLoadoutColor(ColorData);
	}
}
