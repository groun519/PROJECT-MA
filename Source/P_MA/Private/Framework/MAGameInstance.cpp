// Fill out your copyright notice in the Description page of Project Settings.


#include "MAGameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"

void UMAGameInstance::Init()
{
	Super::Init();

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
