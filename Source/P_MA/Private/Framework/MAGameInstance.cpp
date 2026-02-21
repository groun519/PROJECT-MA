// Fill out your copyright notice in the Description page of Project Settings.


#include "MAGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "MoviePlayer.h"
#include "Widget/Lobby/Loading/LoadingScreenWidget.h"
#include "GameFramework/GameStateBase.h"
#include "Player/MAPlayerState.h"
#include "Framework/LoadoutSaveGame.h"

void UMAGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UMAGameInstance::HandlePreLoadMap);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UMAGameInstance::HandlePostLoadMapWithWorld);

	if (bPlayStartupMovie && !StartupMovieName.IsEmpty())
	{
		FLoadingScreenAttributes StartupScreen;
		StartupScreen.MinimumLoadingScreenDisplayTime = 0.0f;
		StartupScreen.bAutoCompleteWhenLoadingCompletes = true;
		StartupScreen.bWaitForManualStop = false;
		StartupScreen.bMoviesAreSkippable = false;
		StartupScreen.bAllowEngineTick = true;
		StartupScreen.MoviePaths.Add(StartupMovieName);
		GetMoviePlayer()->SetupLoadingScreen(StartupScreen);
		GetMoviePlayer()->PlayMovie();
	}

	if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInviteAcceptedHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
				FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UMAGameInstance::HandleSessionInviteAccepted)
			);
		}
	}
}

void UMAGameInstance::Shutdown()
{
	FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	if (LoadingStatusFrameHandle.IsValid())
	{
		FCoreDelegates::OnBeginFrame.Remove(LoadingStatusFrameHandle);
		LoadingStatusFrameHandle.Reset();
	}
	if (MoviePlayerTickHandle.IsValid())
	{
		GetMoviePlayer()->OnMoviePlaybackTick().Remove(MoviePlayerTickHandle);
		MoviePlayerTickHandle.Reset();
	}
	Super::Shutdown();
}

void UMAGameInstance::HostSession(int32 MaxPlayers, bool bIsLAN)
{
	if (!SessionInterface.IsValid()) return;

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		DestroySession();
		return;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = bIsLAN;
	SessionSettings.NumPublicConnections = MaxPlayers;
	SessionSettings.bAllowInvites = true;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	SessionSettings.bAllowJoinViaPresenceFriendsOnly = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUseLobbiesIfAvailable = true;

	CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UMAGameInstance::HandleCreateSessionComplete)
	);

	SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);
}

void UMAGameInstance::StartSession()
{
	if (!SessionInterface.IsValid()) return;

	StartSessionCompleteHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(
		FOnStartSessionCompleteDelegate::CreateUObject(this, &UMAGameInstance::HandleStartSessionComplete)
	);
	SessionInterface->StartSession(NAME_GameSession);
}

void UMAGameInstance::DestroySession()
{
	if (!SessionInterface.IsValid()) return;

	DestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMAGameInstance::HandleDestroySessionComplete)
	);
	SessionInterface->DestroySession(NAME_GameSession);
}

void UMAGameInstance::StartLoadingScreen()
{
	if (bLoadingScreenActive) return;
	if (!LoadingScreenWidgetClass) return;

	LoadingScreenWidgetInstance = nullptr;
	LoadingScreenSlateWidget.Reset();

	LoadingScreenWidgetInstance = CreateWidget<ULoadingScreenWidget>(this, LoadingScreenWidgetClass);
	if (!LoadingScreenWidgetInstance) return;
	LoadingScreenSlateWidget = LoadingScreenWidgetInstance->TakeWidget();

	LoadingScreenStartTime = FPlatformTime::Seconds();
	bLoadingScreenActive = true;
	LoadingStatusLastUpdateSeconds = FPlatformTime::Seconds();

	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.MinimumLoadingScreenDisplayTime = 0.0f;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = bAutoCompleteLoadingScreen;
	LoadingScreen.bWaitForManualStop = !bAutoCompleteLoadingScreen;
	LoadingScreen.bAllowEngineTick = true;
	LoadingScreen.WidgetLoadingScreen = LoadingScreenSlateWidget;
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	GetMoviePlayer()->PlayMovie();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LoadingStatusTimerHandle);
	}
	if (!LoadingStatusFrameHandle.IsValid())
	{
		LoadingStatusFrameHandle = FCoreDelegates::OnBeginFrame.AddUObject(
			this,
			&UMAGameInstance::HandleBeginFrame
		);
	}
	if (!MoviePlayerTickHandle.IsValid())
	{
		MoviePlayerTickHandle = GetMoviePlayer()->OnMoviePlaybackTick().AddUObject(
			this,
			&UMAGameInstance::HandleMoviePlayerTick
		);
	}
	UpdateLoadingStatus();
}

void UMAGameInstance::StopLoadingScreen()
{
	if (!bLoadingScreenActive) return;

	bLoadingScreenActive = false;
	GetMoviePlayer()->StopMovie();
	LoadingScreenSlateWidget.Reset();
	LoadingScreenWidgetInstance = nullptr;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LoadingStatusTimerHandle);
	}
	if (LoadingStatusFrameHandle.IsValid())
	{
		FCoreDelegates::OnBeginFrame.Remove(LoadingStatusFrameHandle);
		LoadingStatusFrameHandle.Reset();
	}
	if (MoviePlayerTickHandle.IsValid())
	{
		GetMoviePlayer()->OnMoviePlaybackTick().Remove(MoviePlayerTickHandle);
		MoviePlayerTickHandle.Reset();
	}
}

void UMAGameInstance::HandlePreLoadMap(const FString& MapName)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LoadingStatusTimerHandle);
	}
}

float UMAGameInstance::CalculateLoadingProgress(int32& OutPercent)
{
	OutPercent = 0;

	const UWorld* World = GetWorld();
	if (!World) return 0.0f;

	const AGameStateBase* GS = World->GetGameState<AGameStateBase>();
	if (!GS) return 0.0f;

	int32 ValidPlayers = 0;
	int32 LoadedPlayers = 0;
	for (APlayerState* PS : GS->PlayerArray)
	{
		const AMAPlayerState* MAPlayerState = Cast<AMAPlayerState>(PS);
		if (!MAPlayerState)
		{
			continue;
		}

		++ValidPlayers;
		if (MAPlayerState->IsLoadingComplete())
		{
			++LoadedPlayers;
		}
	}

	if (ValidPlayers <= 0) return 0.0f;

	const float Progress = FMath::Clamp(static_cast<float>(LoadedPlayers) / static_cast<float>(ValidPlayers), 0.0f, 1.0f);
	OutPercent = FMath::RoundToInt(Progress * 100.0f);

	return Progress;
}

void UMAGameInstance::HandlePostLoadMapWithWorld(UWorld* LoadedWorld)
{
	if (!bLoadingScreenActive || !LoadedWorld) return;

	LoadingScreenStartTime = FPlatformTime::Seconds();
	LoadedWorld->GetTimerManager().ClearTimer(LoadingStatusTimerHandle);
	LoadedWorld->GetTimerManager().SetTimer(
		LoadingStatusTimerHandle,
		this,
		&UMAGameInstance::UpdateLoadingStatus,
		0.2f,
		true
	);
	UpdateLoadingStatus();
}

void UMAGameInstance::HandleBeginFrame()
{
	if (!bLoadingScreenActive) return;

	if (LoadingScreenSlateWidget.IsValid() && !GetMoviePlayer()->IsMovieCurrentlyPlaying())
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.MinimumLoadingScreenDisplayTime = 0.0f;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
		LoadingScreen.bWaitForManualStop = true;
		LoadingScreen.bAllowEngineTick = true;
		LoadingScreen.WidgetLoadingScreen = LoadingScreenSlateWidget;
		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
		GetMoviePlayer()->PlayMovie();
	}

	const double Now = FPlatformTime::Seconds();
	if ((Now - LoadingStatusLastUpdateSeconds) >= 0.2)
	{
		LoadingStatusLastUpdateSeconds = Now;
		UpdateLoadingStatus();
	}

}

void UMAGameInstance::HandleMoviePlayerTick(float DeltaTime)
{
	if (!bLoadingScreenActive) return;

	const double Now = FPlatformTime::Seconds();
	if ((Now - LoadingStatusLastUpdateSeconds) >= 0.2)
	{
		LoadingStatusLastUpdateSeconds = Now;
		UpdateLoadingStatus();
	}

}

void UMAGameInstance::UpdateLoadingStatus()
{
	if (!bLoadingScreenActive) return;

	UWorld* World = GetWorld();
	if (!World) return;

	AGameStateBase* GS = World->GetGameState<AGameStateBase>();
	if (!GS) return;

	TArray<FLoadingPlayerStatus> Statuses;
	Statuses.Reserve(GS->PlayerArray.Num());

	int32 ValidPlayers = 0;
	int32 LoadedPlayers = 0;
	for (APlayerState* PS : GS->PlayerArray)
	{
		AMAPlayerState* MAPlayerState = Cast<AMAPlayerState>(PS);
		if (!MAPlayerState) continue;
		++ValidPlayers;

		FLoadingPlayerStatus Status;
		Status.PlayerName = MAPlayerState->GetPlayerName();
		Status.SlotIndex = MAPlayerState->GetLobbySlotIndex();
		Status.bLoaded = MAPlayerState->IsLoadingComplete();
		if (Status.bLoaded)
		{
			++LoadedPlayers;
		}
		const FMaterialParamDataPair& Colors = MAPlayerState->GetLoadoutColor();
		Status.BodyColor = Colors.BodyData.Color;
		Status.EyeColor = Colors.EyeData.Color;
		Statuses.Add(Status);

	}

	Statuses.Sort([](const FLoadingPlayerStatus& A, const FLoadingPlayerStatus& B)
	{
		return A.SlotIndex < B.SlotIndex;
	});

	const bool bAllLoaded = AreAllPlayersLoaded(World);
	if (LoadingScreenWidgetInstance)
	{
		const float Target = (ValidPlayers > 0)
			? FMath::Clamp(static_cast<float>(LoadedPlayers) / static_cast<float>(ValidPlayers), 0.0f, 1.0f)
			: 0.0f;
		const bool bLoadingComplete = (Target >= 1.0f) || bAllLoaded;
		const float WarmupDurationSeconds = 5.0f;
		const float WarmupMax = 0.50f;
		const float MainMax = 0.95f;
		LoadingScreenWidgetInstance->UpdateLoadingProgress(
			Target,
			bLoadingComplete,
			LoadingFinishDurationSeconds,
			WarmupDurationSeconds,
			WarmupMax,
			MainMax
		);
		LoadingScreenWidgetInstance->UpdateLoadingStatus(Statuses);
	}

	if (bAllLoaded)
	{
		StopLoadingScreen();
	}

}

void UMAGameInstance::SaveLoadout(const FMaterialParamDataPair& Color, FName WeaponId)
{
	if (LoadoutSaveSlot.IsEmpty()) return;

	ULoadoutSaveGame* SaveGame = Cast<ULoadoutSaveGame>(UGameplayStatics::CreateSaveGameObject(ULoadoutSaveGame::StaticClass()));
	if (!SaveGame) return;

	SaveGame->SavedColor = Color;
	SaveGame->SavedWeaponId = WeaponId;

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, LoadoutSaveSlot, LoadoutSaveUserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadoutSave: SaveGameToSlot failed. Slot=%s"), *LoadoutSaveSlot);
	}
}

bool UMAGameInstance::LoadLoadout(FMaterialParamDataPair& OutColor, FName& OutWeaponId)
{
	if (LoadoutSaveSlot.IsEmpty()) return false;

	if (!UGameplayStatics::DoesSaveGameExist(LoadoutSaveSlot, LoadoutSaveUserIndex)) return false;

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(LoadoutSaveSlot, LoadoutSaveUserIndex);
	ULoadoutSaveGame* SaveGame = Cast<ULoadoutSaveGame>(Loaded);
	if (!SaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadoutSave: LoadGameFromSlot returned invalid data. Slot=%s"), *LoadoutSaveSlot);
		return false;
	}

	OutColor = SaveGame->SavedColor;
	OutWeaponId = SaveGame->SavedWeaponId;
	return true;
}

bool UMAGameInstance::AreAllPlayersLoaded(UWorld* World) const
{
	if (!World) return false;

	const AGameStateBase* GS = World->GetGameState<AGameStateBase>();
	if (!GS) return false;

	int32 Total = 0;
	int32 Loaded = 0;
	for (APlayerState* PS : GS->PlayerArray)
	{
		AMAPlayerState* MAPlayerState = Cast<AMAPlayerState>(PS);
		if (!MAPlayerState) continue;

		++Total;
		if (MAPlayerState->IsLoadingComplete())
		{
			++Loaded;
		}
	}

	return Total > 0 && Loaded == Total;
}

void UMAGameInstance::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);

	if (!bWasSuccessful) return;

	const FString LobbyMapPath = TEXT("/Game/_Map/LobbyMap");
	UGameplayStatics::OpenLevel(this, FName(*LobbyMapPath), true, TEXT("listen"));
}

void UMAGameInstance::HandleStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteHandle);
}

void UMAGameInstance::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
}

void UMAGameInstance::HandleSessionInviteAccepted(
	const bool bWasSuccessful,
	int32 ControllerId,
	TSharedPtr<const FUniqueNetId> UserId,
	const FOnlineSessionSearchResult& InviteResult
)
{
	if (!bWasSuccessful || !SessionInterface.IsValid()) return;

	JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMAGameInstance::HandleJoinSessionComplete)
	);
	SessionInterface->JoinSession(ControllerId, NAME_GameSession, InviteResult);
}

void UMAGameInstance::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);

	if (Result != EOnJoinSessionCompleteResult::Success) return;

	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
		{
			PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
		}
	}
}
