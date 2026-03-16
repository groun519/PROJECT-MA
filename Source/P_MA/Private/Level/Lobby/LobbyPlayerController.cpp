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
#include "Widget/Lobby/Loadout/LoadoutWidget.h"
#include "Widget/System/SystemMenuWidget.h"
#include "Framework/MAGameInstance.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Player/Loadout/Data/LoadoutWeaponData.h"
#include "Player/MAPlayerState.h"
#include "Level/Lobby/LobbyAvatarSlot.h"

void ALobbyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController())
	{
		ShowLobbyUI();

		bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
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

		if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
		{
			FLoadoutSelection LoadedLoadout;
			if (GI->LoadLoadout(LoadedLoadout))
			{
				if (HasAuthority())
				{
					if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
					{
						PS->SetLoadoutSelection(LoadedLoadout);
					}
				}
				else
				{
					ServerSetLoadoutSelection(LoadedLoadout);
				}

				PendingLoadout = LoadedLoadout;
				bHasPendingLoadout = true;
			}
		}

		TArray<AActor*> TaggedActors;
		UGameplayStatics::GetAllActorsWithTag(this, LobbyCameraTag, TaggedActors);
		if (TaggedActors.Num() > 0 && TaggedActors[0])
		{
			LobbyCameraActor = TaggedActors[0];
			SetViewTargetWithBlend(LobbyCameraActor, 0.0f);
			LobbyCameraTransform = LobbyCameraActor->GetActorTransform();
			TargetCameraTransform = LobbyCameraTransform * LobbyView.Offset;

			LobbyCameraComponent = LobbyCameraActor->FindComponentByClass<UCameraComponent>();
			if (LobbyCameraComponent)
			{
				if (LobbyView.Fov > 0.0f)
				{
					LobbyCameraComponent->SetFieldOfView(LobbyView.Fov);
				}
				TargetFov = LobbyCameraComponent->FieldOfView;
			}
			ActiveViewSettings = LobbyView;
			bUseCameraInterp = ActiveViewSettings.bUseInterp;

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
	GetWorldTimerManager().ClearTimer(WeaponPreviewTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ALobbyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocalController() || !LobbyCameraActor) return;
	if (!bUseCameraInterp || bIsCameraFading) return;

	if (ActiveViewSettings.CameraInterpSpeed > 0.0f)
	{
		const FVector CurLocation = LobbyCameraActor->GetActorLocation();
		const FRotator CurRotation = LobbyCameraActor->GetActorRotation();
		const FVector NewLocation = FMath::VInterpTo(
			CurLocation,
			TargetCameraTransform.GetLocation(),
			DeltaTime,
			ActiveViewSettings.CameraInterpSpeed
		);
		const FRotator NewRotation = FMath::RInterpTo(
			CurRotation,
			TargetCameraTransform.Rotator(),
			DeltaTime,
			ActiveViewSettings.CameraInterpSpeed
		);
		LobbyCameraActor->SetActorLocationAndRotation(NewLocation, NewRotation);
	}

	if (LobbyCameraComponent && ActiveViewSettings.FovInterpSpeed > 0.0f)
	{
		const float NewFov = FMath::FInterpTo(
			LobbyCameraComponent->FieldOfView,
			TargetFov,
			DeltaTime,
			ActiveViewSettings.FovInterpSpeed
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
	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
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

void ALobbyPlayerController::PreviewEyeColor(const FMaterialParamData& EyeData)
{
	EnsurePendingLoadoutInitialized();

	PendingLoadout.Color.EyeData = EyeData;
	ApplyPreviewColor(PendingLoadout.Color);
}

void ALobbyPlayerController::PreviewEyeShape(FName EyeShapeId)
{
	EnsurePendingLoadoutInitialized();
	PendingLoadout.EyeShapeId = EyeShapeId;

	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		const int32 SlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
		if (ALobbyAvatarSlot* Slot = LGS->GetAvatarSlot(SlotIndex))
		{
			Slot->ApplyLoadoutEyeShape(PendingLoadout.EyeShapeId);
		}
	}
}

void ALobbyPlayerController::PreviewBodyColor(const FMaterialParamData& BodyData)
{
	EnsurePendingLoadoutInitialized();

	PendingLoadout.Color.BodyData = BodyData;
	ApplyPreviewColor(PendingLoadout.Color);
}

void ALobbyPlayerController::PreviewWeapon(FName WeaponId, USkeletalMesh* Mesh, const FTransform& Offset)
{
	EnsurePendingLoadoutInitialized();
	PendingLoadout.WeaponId = WeaponId;

	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		const int32 SlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
		if (ALobbyAvatarSlot* Slot = LGS->GetAvatarSlot(SlotIndex))
		{
			Slot->ApplyLoadoutWeaponMesh(Mesh, Offset);
			Slot->SetWeaponPreviewVisible(true);
		}
	}
}

void ALobbyPlayerController::SetPendingMount(FName MountId)
{
	EnsurePendingLoadoutInitialized();
	PendingLoadout.MountId = MountId;

	if (bInLoadoutView && CurrentLoadoutView == ELoadoutView::Mount)
	{
		ApplyPendingMountPreview();
	}
}

void ALobbyPlayerController::ApplyPendingWeaponPreview()
{
	if (PendingLoadout.WeaponId.IsNone())
	{
		return;
	}

	const FName WeaponId = PendingLoadout.WeaponId;

	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	const UDataTable* WeaponDataTable = LoadoutDataSet ? LoadoutDataSet->WeaponDataTable : nullptr;
	if (!WeaponDataTable)
	{
		return;
	}

	const FLoadoutWeaponDataRow* Row = WeaponDataTable->FindRow<FLoadoutWeaponDataRow>(WeaponId, TEXT("ApplyPendingWeaponPreview"));
	if (!Row)
	{
		return;
	}

	USkeletalMesh* Mesh = Row->WeaponMesh.LoadSynchronous();
	PreviewWeapon(WeaponId, Mesh, Row->WeaponOffset);
}

void ALobbyPlayerController::ApplyPendingMountPreview()
{
	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		const int32 SlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
		if (ALobbyAvatarSlot* Slot = LGS->GetAvatarSlot(SlotIndex))
		{
			Slot->ApplyLoadoutMountId(PendingLoadout.MountId);
			Slot->SetMountPreviewVisible(true);
		}
	}
}

void ALobbyPlayerController::EnsurePendingLoadoutInitialized()
{
	if (bHasPendingLoadout)
	{
		return;
	}

	if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
	{
		PendingLoadout = PS->GetLoadoutSelection();
		bHasPendingLoadout = true;
	}
}

void ALobbyPlayerController::ApplyPendingWeaponPreviewDelayed(float DelaySeconds)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(WeaponPreviewTimerHandle);
	World->GetTimerManager().SetTimer(
		WeaponPreviewTimerHandle,
		this,
		&ALobbyPlayerController::ApplyPendingWeaponPreview,
		FMath::Max(0.f, DelaySeconds),
		false
	);
}

void ALobbyPlayerController::HandleReadyStartClicked()
{
	const bool bIsHost = HasAuthority() && IsLocalController();
	if (bIsHost)
	{
		if (LobbyRootWidgetInstance)
		{
			if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
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
						UE_LOG(LogTemp, Warning, TEXT("Lobby: Host starting ServerTravel to /Game/_Map/MainMap?listen"));
						for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
						{
							if (ALobbyPlayerController* PC = Cast<ALobbyPlayerController>(It->Get()))
							{
								PC->ClientStartLoadingScreen();
							}
						}
						World->ServerTravel(TEXT("/Game/_Map/MainMap?listen"));
					}
				}
			}
		}
		return;
	}

	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		const bool bIsReady = LGS->IsPlayerReady(GetPlayerState<APlayerState>());
		SetReady(!bIsReady);
	}
}

void ALobbyPlayerController::ClientStartLoadingScreen_Implementation()
{
	if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
	{
		GI->StartLoadingScreen();
	}
}

void ALobbyPlayerController::HandleLoadoutClicked()
{
	if (!IsLocalController()) return;

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

	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
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
	const FLoadoutCameraViewSettings PrevViewSettings = ActiveViewSettings;
	bInLoadoutView = true;
	CurrentLoadoutView = ELoadoutView::Body;
	UpdateCameraTarget();

	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
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

		ServerSetLobbyState(ELobbyAvatarState::Loadout);
	}

	if (LobbyRootWidgetInstance && LobbyRootWidgetInstance->LoadoutButtonText)
	{
		LobbyRootWidgetInstance->LoadoutButtonText->SetText(FText::FromString(TEXT("Save")));
	}

	EnsurePendingLoadoutInitialized();

	if (LobbyRootWidgetInstance && LobbyRootWidgetInstance->LoadoutWidget)
	{
		LobbyRootWidgetInstance->LoadoutWidget->ActivateBodyTabUI();
		LobbyRootWidgetInstance->LoadoutWidget->SyncSelectionFromPending(PendingLoadout);
		LobbyRootWidgetInstance->LoadoutWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	ApplyCameraTransition(PrevViewSettings, ActiveViewSettings);
}

void ALobbyPlayerController::SetLoadoutView(ELoadoutView NewView)
{
	if (!bInLoadoutView) return;

	const FLoadoutCameraViewSettings PrevViewSettings = ActiveViewSettings;
	CurrentLoadoutView = NewView;

	if (CurrentLoadoutView != ELoadoutView::Weapon)
	{
		GetWorldTimerManager().ClearTimer(WeaponPreviewTimerHandle);
		SetLocalSlotWeaponPreviewVisible(false);
	}

	if (CurrentLoadoutView == ELoadoutView::Mount)
	{
		ApplyPendingMountPreview();
	}
	else
	{
		SetLocalSlotMountPreviewVisible(false);
	}

	UpdateCameraTarget();

	ApplyCameraTransition(PrevViewSettings, ActiveViewSettings);
}

void ALobbyPlayerController::ExitLoadoutView()
{
	const FLoadoutCameraViewSettings PrevViewSettings = ActiveViewSettings;
	CommitPendingLoadout();
	GetWorldTimerManager().ClearTimer(WeaponPreviewTimerHandle);
	SetLocalSlotMountPreviewVisible(false);
	SetLocalSlotWeaponPreviewVisible(true);

	if (IsLocalController())
	{
		if (UMAGameInstance* GI = GetGameInstance<UMAGameInstance>())
		{
			EnsurePendingLoadoutInitialized();
			GI->SaveLoadout(PendingLoadout);
		}
	}
	bInLoadoutView = false;
	UpdateCameraTarget();

	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		const TArray<TObjectPtr<ALobbyAvatarSlot>>& Slots = LGS->GetAvatarSlots();
		for (ALobbyAvatarSlot* Slot : Slots)
		{
			if (Slot)
			{
				Slot->SetLocalHidden(false);
			}
		}

		const bool bIsReady = LGS->IsPlayerReady(GetPlayerState<APlayerState>());
		const int32 SlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
		if (ALobbyAvatarSlot* Slot = LGS->GetAvatarSlot(SlotIndex))
		{
			Slot->SetLobbyState(bIsReady ? ELobbyAvatarState::Ready : ELobbyAvatarState::Wait);
		}
		ServerSetLobbyState(bIsReady ? ELobbyAvatarState::Ready : ELobbyAvatarState::Wait);
	}

	if (LobbyRootWidgetInstance && LobbyRootWidgetInstance->LoadoutButtonText)
	{
		LobbyRootWidgetInstance->LoadoutButtonText->SetText(FText::FromString(TEXT("Loadout")));
	}

	if (LobbyRootWidgetInstance && LobbyRootWidgetInstance->LoadoutWidget)
	{
		LobbyRootWidgetInstance->LoadoutWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	ApplyCameraTransition(PrevViewSettings, ActiveViewSettings);
}

void ALobbyPlayerController::UpdateCameraTarget()
{
	if (!LobbyCameraActor) return;

	if (!bInLoadoutView)
	{
		TargetCameraTransform = LobbyCameraTransform * LobbyView.Offset;
		ActiveViewSettings = LobbyView;

		if (LobbyCameraComponent && ActiveViewSettings.Fov > 0.0f)
		{
			TargetFov = ActiveViewSettings.Fov;
		}

		return;
	}

	ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>();
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
	FLoadoutCameraViewSettings ViewSettings = LoadoutBodyView;
	if (CurrentLoadoutView == ELoadoutView::Head)
	{
		ViewSettings = LoadoutHeadView;
	}
	else if (CurrentLoadoutView == ELoadoutView::Weapon)
	{
		ViewSettings = LoadoutWeaponView;
	}
	else if (CurrentLoadoutView == ELoadoutView::Mount)
	{
		ViewSettings = LoadoutMountView;
	}

	const FVector WorldLocation = SlotTransform.TransformPosition(ViewSettings.Offset.GetLocation());
	const FQuat WorldRotation = SlotTransform.GetRotation() * ViewSettings.Offset.GetRotation();
	TargetCameraTransform = FTransform(WorldRotation, WorldLocation, FVector::OneVector);

	if (LobbyCameraComponent && ViewSettings.Fov > 0.0f)
	{
		TargetFov = ViewSettings.Fov;
	}

	ActiveViewSettings = ViewSettings;
}

void ALobbyPlayerController::ApplyCameraTransition(const FLoadoutCameraViewSettings& PrevViewSettings, const FLoadoutCameraViewSettings& NextViewSettings)
{
	const bool bUseFade = !PrevViewSettings.bUseInterp || !NextViewSettings.bUseInterp;
	if (bUseFade)
	{
		const FLoadoutCameraViewSettings& FadeSettings = NextViewSettings.bUseInterp ? PrevViewSettings : NextViewSettings;
		ApplyFadeTransition(FadeSettings);
	}
	else
	{
		ApplyInterpTransition();
	}
}

void ALobbyPlayerController::ApplyFadeTransition(const FLoadoutCameraViewSettings& NextViewSettings)
{
	bUseCameraInterp = false;
	TriggerInstantCameraFade(NextViewSettings);
}

void ALobbyPlayerController::ApplyInterpTransition()
{
	bIsCameraFading = false;
	bUseCameraInterp = true;
}

void ALobbyPlayerController::ApplyInstantCameraTarget()
{
	if (!LobbyCameraActor) return;

	LobbyCameraActor->SetActorLocationAndRotation(
		TargetCameraTransform.GetLocation(),
		TargetCameraTransform.Rotator()
	);

	if (LobbyCameraComponent && TargetFov > 0.0f)
	{
		LobbyCameraComponent->SetFieldOfView(TargetFov);
	}
}

void ALobbyPlayerController::TriggerInstantCameraFade(const FLoadoutCameraViewSettings& ViewSettings)
{
	bIsCameraFading = true;

	if (!PlayerCameraManager)
	{
		ApplyInstantCameraTarget();
		bIsCameraFading = false;
		return;
	}

	const float FadeOut = FMath::Max(0.0f, ViewSettings.FadeSeconds.CameraFadeOutSeconds);
	const float FadeIn = FMath::Max(0.0f, ViewSettings.FadeSeconds.CameraFadeInSeconds);
	if (FadeOut <= 0.0f && FadeIn <= 0.0f)
	{
		bIsCameraFading = false;
		return;
	}

	GetWorldTimerManager().ClearTimer(CameraFadeTimerHandle);
	GetWorldTimerManager().ClearTimer(CameraFadeEndTimerHandle);

	if (FadeOut > 0.0f)
	{
		PlayerCameraManager->StartCameraFade(0.0f, 1.0f, FadeOut, FLinearColor::Black, false, true);
		FTimerDelegate FadeInDelegate;
		FadeInDelegate.BindLambda([this, FadeIn]()
		{
			ApplyInstantCameraTarget();
			if (PlayerCameraManager && FadeIn > 0.0f)
			{
				PlayerCameraManager->StartCameraFade(1.0f, 0.0f, FadeIn, FLinearColor::Black, false, false);
			}
		});
		GetWorldTimerManager().SetTimer(CameraFadeTimerHandle, FadeInDelegate, FadeOut, false);
		GetWorldTimerManager().SetTimer(
			CameraFadeEndTimerHandle,
			[this]()
			{
				bIsCameraFading = false;
			},
			FadeOut + FMath::Max(0.0f, FadeIn),
			false
		);
		return;
	}

	ApplyInstantCameraTarget();

	if (FadeIn > 0.0f)
	{
		PlayerCameraManager->StartCameraFade(1.0f, 0.0f, FadeIn, FLinearColor::Black, false, false);
		GetWorldTimerManager().SetTimer(
			CameraFadeEndTimerHandle,
			[this]()
			{
				bIsCameraFading = false;
			},
			FadeIn,
			false
		);
		return;
	}

	bIsCameraFading = false;
}

void ALobbyPlayerController::ApplyPreviewColor(const FMaterialParamDataPair& ColorData)
{
	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		const int32 SlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
		if (ALobbyAvatarSlot* Slot = LGS->GetAvatarSlot(SlotIndex))
		{
			Slot->ApplyLoadoutColor(ColorData);
		}
	}
}

void ALobbyPlayerController::SetLocalSlotMountPreviewVisible(bool bVisible)
{
	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		const int32 SlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
		if (ALobbyAvatarSlot* Slot = LGS->GetAvatarSlot(SlotIndex))
		{
			Slot->SetMountPreviewVisible(bVisible);
		}
	}
}

void ALobbyPlayerController::SetLocalSlotWeaponPreviewVisible(bool bVisible)
{
	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		const int32 SlotIndex = LGS->GetSlotIndex(GetPlayerState<APlayerState>());
		if (ALobbyAvatarSlot* Slot = LGS->GetAvatarSlot(SlotIndex))
		{
			Slot->SetWeaponPreviewVisible(bVisible);
		}
	}
}

void ALobbyPlayerController::CommitPendingLoadout()
{
	if (!bHasPendingLoadout) return;

	if (HasAuthority())
	{
		if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
		{
			PS->SetLoadoutSelection(PendingLoadout);
		}
	}
	else
	{
		ServerSetLoadoutSelection(PendingLoadout);
	}
}

void ALobbyPlayerController::ServerSetLoadoutSelection_Implementation(const FLoadoutSelection& Loadout)
{
	if (AMAPlayerState* PS = GetPlayerState<AMAPlayerState>())
	{
		PS->SetLoadoutSelection(Loadout);
	}
}

void ALobbyPlayerController::ServerSetLobbyState_Implementation(ELobbyAvatarState NewState)
{
	if (ALobbyGameState* LGS = GetWorld()->GetGameState<ALobbyGameState>())
	{
		LGS->SetPlayerLobbyState(GetPlayerState<APlayerState>(), NewState);
	}
}
