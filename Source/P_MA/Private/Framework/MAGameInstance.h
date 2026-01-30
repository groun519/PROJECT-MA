// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MAGameInstance.generated.h"





/**
 * 
 */
UCLASS()
class P_MA_API UMAGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Online")
	void HostSession(int32 MaxPlayers, bool bIsLAN);

	UFUNCTION(BlueprintCallable, Category = "Online")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "Online")
	void DestroySession();

private:
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
};
