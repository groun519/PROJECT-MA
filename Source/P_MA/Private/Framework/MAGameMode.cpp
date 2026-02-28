// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MAGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Player/MAPlayerState.h"
#include "Framework/MAGameState.h"
#include "Framework/ReadyManagerComponent.h"

AMAGameMode::AMAGameMode()
{
	CurrentMASectorState = EMASectorState::Wait;
	GameStateClass = AMAGameState::StaticClass();
	ReadyManagerComponent = CreateDefaultSubobject<UReadyManagerComponent>(TEXT("ReadyManagerComponent"));
}

APlayerController* AMAGameMode::SpawnPlayerController(ENetRole InRemoteRole, const FString& Options)
{
	APlayerController* NewPlayerController = Super::SpawnPlayerController(InRemoteRole, Options);
	IGenericTeamAgentInterface* NewPlayerTeamInterface = Cast<IGenericTeamAgentInterface>(NewPlayerController);
	FGenericTeamId TeamId = FGenericTeamId(0);
	if (NewPlayerTeamInterface)
	{
		NewPlayerTeamInterface->SetGenericTeamId(TeamId);
	}

	NewPlayerController->StartSpot = FIndNextStartSpotForTeam(TeamId);
	return NewPlayerController;
}

void AMAGameMode::BeginPlay()
{
	Super::BeginPlay();
	if (AMAGameState* GS = GetGameState<AMAGameState>())
	{
		GS->SetMASectorState(CurrentMASectorState);
	}

	ReadyManagerComponent->OnReadyCountsChanged.RemoveAll(this);
	ReadyManagerComponent->OnAllPlayersReadyChanged.RemoveAll(this);
	ReadyManagerComponent->OnReadyCountsChanged.AddUObject(this, &AMAGameMode::HandleReadyCountsChanged);
	ReadyManagerComponent->OnAllPlayersReadyChanged.AddUObject(this, &AMAGameMode::HandleAllPlayersReadyChanged);

	RefreshPlayerCache();

	const UWorld* World = GetWorld();
	const FString MapName = World ? World->GetMapName() : FString();
	const bool bResetLoadingState = MapName.Contains(TEXT("LobbyMap"));
	if (bResetLoadingState && GameState)
	{
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (AMAPlayerState* MAPlayerState = Cast<AMAPlayerState>(PS))
			{
				MAPlayerState->SetLoadingComplete(false);
			}
		}
	}
}

void AMAGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	const UWorld* World = GetWorld();
	const FString MapName = World ? World->GetMapName() : FString();
	const bool bResetLoadingState = MapName.Contains(TEXT("LobbyMap"));
	if (bResetLoadingState && NewPlayer)
	{
		if (AMAPlayerState* MAPlayerState = Cast<AMAPlayerState>(NewPlayer->PlayerState))
		{
			MAPlayerState->SetLoadingComplete(false);
		}
	}
	RefreshPlayerCache();
}

void AMAGameMode::RequestStateChange(EMASectorState NewState)
{
	if (CurrentMASectorState == NewState) return;

	CurrentMASectorState = NewState;
	if (AMAGameState* GS = GetGameState<AMAGameState>())
	{
		GS->SetMASectorState(CurrentMASectorState);
	}

	if (OnMASectorStateChanged.IsBound())
	{
		OnMASectorStateChanged.Broadcast(CurrentMASectorState);
	}
}

EMASectorState AMAGameMode::GetNextState(EMASectorState CurState) const
{
	if (CurState == EMASectorState::Loop)
	{
		return EMASectorState::Start;
	}

	return static_cast<EMASectorState>(static_cast<int32>(CurState) + 1);
}

void AMAGameMode::RequestNextState(EMASectorState CurState)
{
	RequestStateChange(GetNextState(CurState));
}

void AMAGameMode::RefreshPlayerCache()
{
	ReadyManagerComponent->RefreshPlayerCache();
}

void AMAGameMode::ResetAllPlayersReady()
{
	ReadyManagerComponent->ResetAllPlayersReady();
}

void AMAGameMode::GetReadyCounts(int32& OutReady, int32& OutTotal) const
{
	OutReady = 0;
	OutTotal = 0;
	ReadyManagerComponent->GetReadyCounts(OutReady, OutTotal);
}

void AMAGameMode::BroadcastReadyCounts()
{
	ReadyManagerComponent->BroadcastReadyCounts();
}

void AMAGameMode::SetPlayerLoopReady(APlayerState* PlayerState, bool bReady)
{
	ReadyManagerComponent->SetPlayerLoopReady(PlayerState, bReady);
}

bool AMAGameMode::IsPlayerLoopReady(const APlayerState* PlayerState) const
{
	return ReadyManagerComponent->IsPlayerLoopReady(PlayerState);
}


void AMAGameMode::SetMAState(int32 NewState)
{
	if (NewState < 0 || NewState > static_cast<int32>(EMASectorState::Loop))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetMAState: invalid state %d"), NewState);
		return;
	}

	RequestStateChange(static_cast<EMASectorState>(NewState));
}

AActor* AMAGameMode::FIndNextStartSpotForTeam(const FGenericTeamId& TeamID) const
{
	const FName* StartSpotTag = TeamStartSpotTagMap.Find(TeamID);
	if (!StartSpotTag)
	{
		return nullptr;
	}

	UWorld* World = GetWorld();
	
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == *StartSpotTag)
		{
			It->PlayerStartTag = FName("Taken");
			return *It;
		}
	}

	return nullptr;
}

void AMAGameMode::HandleReadyCountsChanged(int32 ReadyCount, int32 TotalCount)
{
	OnReadyCountChanged.Broadcast(ReadyCount, TotalCount);
}

void AMAGameMode::HandleAllPlayersReadyChanged(bool bIsAllReady)
{
	bAllPlayersReady = bIsAllReady;
	if (bAllPlayersReady) OnAllPlayersReady.Broadcast();
}
