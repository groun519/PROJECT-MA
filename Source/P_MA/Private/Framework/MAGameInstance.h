// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Widgets/SWidget.h"
#include "Misc/CoreDelegates.h"
#include "MAGameInstance.generated.h"

class ULoadingScreenWidget;





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

	float CalculateLoadingProgress(int32& OutPercent);
	float GetLoadingFinishDurationSeconds() const { return LoadingFinishDurationSeconds; }
	float GetLoadingPostLoadHoldSeconds() const { return LoadingScreenPostLoadHoldSeconds; }

private:
	void HandlePreLoadMap(const FString& MapName);
	void HandlePostLoadMapWithWorld(UWorld* LoadedWorld);
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
	float LoadingScreenPostLoadHoldSeconds = 0.5f;

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
};
