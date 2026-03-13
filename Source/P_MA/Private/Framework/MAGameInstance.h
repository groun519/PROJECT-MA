// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Widgets/SWidget.h"
#include "Misc/CoreDelegates.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "MAGameInstance.generated.h"

class ULoadingScreenWidget;
class ULoadoutDataSet;

/**
 * 
 */
UCLASS()
class P_MA_API UMAGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

	UFUNCTION(BlueprintCallable, Category = "Online")
	void HostSession(int32 MaxPlayers, bool bIsLAN);

	UFUNCTION(BlueprintCallable, Category = "Online")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Online")
	void DestroySession();

	UFUNCTION(BlueprintCallable, Category = "Loading")
	void StartLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "Loading")
	void StopLoadingScreen();

	UFUNCTION(BlueprintCallable, Category = "Loading")
	void UpdateLoadingStatus();

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	void SaveLoadout(const FLoadoutSelection& Loadout);

	UFUNCTION(BlueprintCallable, Category = "Loadout")
	bool LoadLoadout(FLoadoutSelection& OutLoadout);

	void NotifyLocalLoadingVisualComplete();

	float CalculateLoadingProgress(int32& OutPercent);
	float GetLoadingFinishDurationSeconds() const { return LoadingFinishDurationSeconds; }
	const ULoadoutDataSet* TryGetLoadoutDataSet() const;

private:
	void HandlePreLoadMap(const FString& MapName);
	void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);
	void StartLocalMainMapFinishPhase(UWorld* LoadedWorld, const FString& LoadedMapName);
	bool TrySendLocalLoadedNotify();
	void TryHostLobbySession(UWorld* LoadedWorld);
	bool AreAllPlayersLoaded(UWorld* World) const;
	void HandleBeginFrame();
	void HandleMoviePlayerTick(float DeltaTime);

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleSessionInviteAccepted(
		const bool bWasSuccessful,
		int32 ControllerId,
		TSharedPtr<const FUniqueNetId> UserId,
		const FOnlineSessionSearchResult& InviteResult
	);
	void JoinPendingInviteSession();
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	IOnlineSessionPtr SessionInterface;
	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle StartSessionCompleteHandle;
	FDelegateHandle DestroySessionCompleteHandle;
	FDelegateHandle SessionInviteAcceptedHandle;
	FDelegateHandle JoinSessionCompleteHandle;

	UPROPERTY(EditAnywhere, Category = "Loading")
	TSubclassOf<ULoadingScreenWidget> LoadingScreenWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Loading")
	bool bAutoCompleteLoadingScreen = true;

	UPROPERTY(EditAnywhere, Category = "Loading")
	float LoadingFinishDurationSeconds = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Startup Movie")
	bool bPlayStartupMovie = true;

	UPROPERTY(EditAnywhere, Category = "Startup Movie")
	FString StartupMovieName = TEXT("InitMovie");

	UPROPERTY()
	TObjectPtr<ULoadingScreenWidget> LoadingScreenWidgetInstance;
	TSharedPtr<SWidget> LoadingScreenSlateWidget;

	FTimerHandle LoadingStatusTimerHandle;
	FDelegateHandle LoadingStatusFrameHandle;
	FDelegateHandle MoviePlayerTickHandle;
	double LoadingStatusLastUpdateSeconds = 0.0;
	double LoadingScreenStartTime = 0.0;
	bool bLoadingScreenActive = false;
	bool bLocalMainMapLoaded = false;
	bool bLocalLoadedNotifySent = false;

	UPROPERTY(EditAnywhere, Category = "Online")
	int32 LobbyMaxPlayers = 4;

	UPROPERTY(EditAnywhere, Category = "Online")
	bool bLobbyIsLAN = false;

	bool bLobbyHostRequested = false;
	bool bInviteJoinInProgress = false;
	bool bHasPendingInviteResult = false;
	int32 PendingInviteControllerId = 0;
	FOnlineSessionSearchResult PendingInviteResult;

	UPROPERTY(EditAnywhere, Category = "Loadout")
	FString LoadoutSaveSlot = TEXT("LoadoutSlot");

	UPROPERTY(EditAnywhere, Category = "Loadout")
	int32 LoadoutSaveUserIndex = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Loadout")
	TObjectPtr<ULoadoutDataSet> LoadoutDataSet;
};
