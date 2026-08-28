// Fill out your copyright notice in the Description page of Project Settings.


#include "MAGameInstance.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/GameUserSettings.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "MoviePlayer.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Widget/Lobby/Loading/LoadingScreenWidget.h"
#include "Player/MAPlayerControllerBase.h"
#include "Player/MAPlayerState.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Framework/LoadoutSaveGame.h"

namespace
{
	const TCHAR* GLanguageSettingsSection = TEXT("MA.Localization");
	const TCHAR* GLanguageCultureKey = TEXT("LanguageCulture");
}

/** Lifecycle **/
void UMAGameInstance::Init()
{
	Super::Init();

	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->LoadSettings(false);
		Settings->ApplySettings(false);
	}
	LoadLanguageSetting();

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

/** Localization **/
FString UMAGameInstance::GetCurrentLanguageCulture() const
{
	return NormalizeLanguageCulture(FInternationalization::Get().GetCurrentLanguage()->GetName());
}

void UMAGameInstance::SetCurrentLanguageCulture(const FString& InCulture)
{
	ApplyLanguageSetting(InCulture);
	SaveLanguageSetting(InCulture);
}

void UMAGameInstance::LoadLanguageSetting()
{
	FString Culture;
	GConfig->GetString(GLanguageSettingsSection, GLanguageCultureKey, Culture, GGameUserSettingsIni);
	ApplyLanguageSetting(Culture);
}

void UMAGameInstance::SaveLanguageSetting(const FString& InCulture) const
{
	GConfig->SetString(GLanguageSettingsSection, GLanguageCultureKey, *NormalizeLanguageCulture(InCulture), GGameUserSettingsIni);
	GConfig->Flush(false, GGameUserSettingsIni);
}

void UMAGameInstance::ApplyLanguageSetting(const FString& InCulture) const
{
	FInternationalization::Get().SetCurrentLanguageAndLocale(NormalizeLanguageCulture(InCulture));
}

FString UMAGameInstance::NormalizeLanguageCulture(const FString& InCulture) const
{
	if (InCulture.StartsWith(TEXT("ko"))) return TEXT("ko");
	if (InCulture.StartsWith(TEXT("en"))) return TEXT("en");
	return DefaultLanguageCulture.IsEmpty() ? TEXT("en") : DefaultLanguageCulture;
}

/** Lifecycle **/
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

const ULoadoutDataSet* UMAGameInstance::TryGetLoadoutDataSet() const
{
	return LoadoutDataSet;
}

/** Online **/
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

void UMAGameInstance::ShowInviteUI()
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (!Subsystem) return;

	const IOnlineExternalUIPtr ExternalUI = Subsystem->GetExternalUIInterface();
	if (ExternalUI.IsValid())
	{
		ExternalUI->ShowInviteUI(0);
	}
}

/** Loading **/
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
	bLocalMainMapLoaded = false;
	bLocalLoadingVisualComplete = false;
	bLocalLoadedNotifySent = false;
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
	bLocalMainMapLoaded = false;
	bLocalLoadingVisualComplete = false;
	bLocalLoadedNotifySent = false;
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
	bLocalMainMapLoaded = false;
	bLocalLoadingVisualComplete = false;
	bLocalLoadedNotifySent = false;
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
	if (!LoadedWorld) return;

	TryHostLobbySession(LoadedWorld);
	const FString LoadedMapName = LoadedWorld->GetOutermost()->GetName();
	if (!bLoadingScreenActive) return;

	StartLocalMainMapFinishPhase(LoadedWorld, LoadedMapName);

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

void UMAGameInstance::StartLocalMainMapFinishPhase(UWorld* LoadedWorld, const FString& LoadedMapName)
{
	if (!LoadedWorld) return;
	if (!LoadedMapName.EndsWith(TEXT("/_Map/MainMap"))) return;

	bLocalMainMapLoaded = true;
	bLocalLoadedNotifySent = false;
}

bool UMAGameInstance::TrySendLocalLoadedNotify()
{
	if (bLocalLoadedNotifySent) return true;

	AMAPlayerControllerBase* LocalController = Cast<AMAPlayerControllerBase>(UGameplayStatics::GetPlayerController(this, 0));
	if (!LocalController)
	{
		return false;
	}

	if (!LocalController->IsLocalController())
	{
		return false;
	}

	LocalController->ServerNotifyLoaded();
	bLocalLoadedNotifySent = true;
	return true;
}

void UMAGameInstance::NotifyLocalLoadingVisualComplete()
{
	bLocalLoadingVisualComplete = true;
	if (bLocalMainMapLoaded && !bLocalLoadedNotifySent)
	{
		TrySendLocalLoadedNotify();
	}
}

/** Online **/
void UMAGameInstance::TryHostLobbySession(UWorld* LoadedWorld)
{
	if (!LoadedWorld) return;
	if (!SessionInterface.IsValid()) return;
	if (LoadedWorld->GetNetMode() == NM_Client) return;
	if (SessionInterface->GetNamedSession(NAME_GameSession)) return;
	if (bLobbyHostRequested) return;

	const FString MapName = LoadedWorld->GetOutermost()->GetName();
	if (!MapName.EndsWith(TEXT("/_Map/LobbyMap"))) return;

	bLobbyHostRequested = true;
	HostSession(LobbyMaxPlayers, bLobbyIsLAN);
}

void UMAGameInstance::HandleBeginFrame()
{
	if (!bLoadingScreenActive) return;

	if (bLocalMainMapLoaded && bLocalLoadingVisualComplete && !bLocalLoadedNotifySent)
	{
		TrySendLocalLoadedNotify();
	}

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
		float Target = (ValidPlayers > 0)
			? FMath::Clamp(static_cast<float>(LoadedPlayers) / static_cast<float>(ValidPlayers), 0.0f, 1.0f)
			: 0.0f;
		if (bLocalMainMapLoaded)
		{
			Target = 1.0f;
		}
		const bool bLoadingComplete = bLocalMainMapLoaded || bAllLoaded;
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

/** Loadout **/
void UMAGameInstance::SaveLoadout(const FLoadoutSelection& Loadout)
{
	if (LoadoutSaveSlot.IsEmpty()) return;

	ULoadoutSaveGame* SaveGame = Cast<ULoadoutSaveGame>(UGameplayStatics::CreateSaveGameObject(ULoadoutSaveGame::StaticClass()));
	if (!SaveGame) return;

	SaveGame->SavedLoadout = Loadout;
	// Reserved for future save migration. Current load logic does not branch on version.
	SaveGame->Version = 1;

	if (!UGameplayStatics::SaveGameToSlot(SaveGame, LoadoutSaveSlot, LoadoutSaveUserIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadoutSave: SaveGameToSlot failed. Slot=%s"), *LoadoutSaveSlot);
	}
}

bool UMAGameInstance::LoadLoadout(FLoadoutSelection& OutLoadout)
{
	if (LoadoutSaveSlot.IsEmpty()) return false;

	OutLoadout = FLoadoutSelection();

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(LoadoutSaveSlot, LoadoutSaveUserIndex);
	if (!Loaded)
	{
		return true;
	}

	ULoadoutSaveGame* SaveGame = Cast<ULoadoutSaveGame>(Loaded);
	if (!SaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadoutSave: LoadGameFromSlot returned invalid data. Slot=%s"), *LoadoutSaveSlot);
		return false;
	}

	OutLoadout = SaveGame->SavedLoadout;
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

/** Online **/
void UMAGameInstance::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);

	if (!bWasSuccessful)
	{
		bLobbyHostRequested = false;
		return;
	}

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
	bLobbyHostRequested = false;

	if (!bWasSuccessful && SessionInterface->GetNamedSession(NAME_GameSession))
	{
		bInviteJoinInProgress = false;
		bHasPendingInviteResult = false;
		return;
	}

	if (bInviteJoinInProgress && bHasPendingInviteResult)
	{
		JoinPendingInviteSession();
	}
}

void UMAGameInstance::HandleSessionInviteAccepted(
	const bool bWasSuccessful,
	int32 ControllerId,
	TSharedPtr<const FUniqueNetId> UserId,
	const FOnlineSessionSearchResult& InviteResult
)
{
	if (!bWasSuccessful || !SessionInterface.IsValid()) return;

	bInviteJoinInProgress = true;
	bHasPendingInviteResult = true;
	PendingInviteControllerId = ControllerId;
	PendingInviteResult = InviteResult;

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		DestroySession();
		return;
	}

	JoinPendingInviteSession();
}

void UMAGameInstance::JoinPendingInviteSession()
{
	if (!SessionInterface.IsValid()) return;
	if (!bHasPendingInviteResult)
	{
		bInviteJoinInProgress = false;
		return;
	}

	JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMAGameInstance::HandleJoinSessionComplete)
	);
	SessionInterface->JoinSession(PendingInviteControllerId, NAME_GameSession, PendingInviteResult);
}

void UMAGameInstance::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid()) return;
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
	bInviteJoinInProgress = false;
	bHasPendingInviteResult = false;

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
