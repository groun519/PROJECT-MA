// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MAGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "EngineUtils.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerState.h"
#include "Player/ReadyStateComponent.h"
#include "Framework/MAGameState.h"

AMAGameMode::AMAGameMode()
{
	CurrentMASectorState = EMASectorState::Wait;
	GameStateClass = AMAGameState::StaticClass();
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
		GS->SyncLoopReadyEntries(GameState ? GameState->PlayerArray : TArray<APlayerState*>());
	}
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
	CachedPlayers.Reset();

	if (!GetWorld()) return;

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(PC->GetPawn());
		if (!Player) continue;

		CachedPlayers.Add(Player);
	}

	if (AMAGameState* GS = GetGameState<AMAGameState>())
	{
		GS->SyncLoopReadyEntries(GameState ? GameState->PlayerArray : TArray<APlayerState*>());
	}

	BroadcastReadyCounts();
}

void AMAGameMode::ResetAllPlayersReady()
{
	bAllPlayersReady = false;

	for (TWeakObjectPtr<AMAPlayerCharacter> PlayerPtr : CachedPlayers)
	{
		AMAPlayerCharacter* Player = PlayerPtr.Get();
		if (!Player) continue;

		UReadyStateComponent* ReadyComp = Player->GetReadyComponent();
		if (!ReadyComp) continue;

		ReadyComp->SetReady(false);
	}

	BroadcastReadyCounts();
}

void AMAGameMode::GetReadyCounts(int32& OutReady, int32& OutTotal) const
{
	OutReady = 0;
	OutTotal = 0;

	for (TWeakObjectPtr<AMAPlayerCharacter> PlayerPtr : CachedPlayers)
	{
		AMAPlayerCharacter* Player = PlayerPtr.Get();
		if (!Player) continue;

		UReadyStateComponent* ReadyComp = Player->GetReadyComponent();
		if (!ReadyComp) continue;

		OutTotal++;
		if (ReadyComp->IsReady())
		{
			OutReady++;
		}
	}
}

void AMAGameMode::BroadcastReadyCounts()
{
	int32 ReadyCount = 0;
	int32 TotalCount = 0;
	GetReadyCounts(ReadyCount, TotalCount);

	const bool bIsAllReady = (TotalCount > 0 && ReadyCount == TotalCount);
	if (bAllPlayersReady != bIsAllReady)
	{
		bAllPlayersReady = bIsAllReady;
		if (bAllPlayersReady)
		{
			RequestNextState(CurrentMASectorState);
		}
	}

	if (OnReadyCountChanged.IsBound())
	{
		OnReadyCountChanged.Broadcast(ReadyCount, TotalCount);
	}
}

void AMAGameMode::SetPlayerLoopReady(APlayerState* PlayerState, bool bReady)
{
	if (!PlayerState)
	{
		return;
	}

	if (AMAGameState* GS = GetGameState<AMAGameState>())
	{
		GS->SetLoopReadyForPlayer(PlayerState, bReady);
	}

	int32 ReadyCount = 0;
	int32 TotalCount = 0;
	if (AMAGameState* GS = GetGameState<AMAGameState>())
	{
		GS->GetLoopReadyCounts(ReadyCount, TotalCount);
	}

	const bool bIsAllReady = (TotalCount > 0 && ReadyCount == TotalCount);
	if (bIsAllReady && CurrentMASectorState == EMASectorState::Loop)
	{
		RequestStateChange(EMASectorState::Start);
		if (AMAGameState* GS = GetGameState<AMAGameState>())
		{
			GS->ResetLoopReadyEntries();
		}
	}
}

bool AMAGameMode::IsPlayerLoopReady(const APlayerState* PlayerState) const
{
	if (!PlayerState)
	{
		return false;
	}

	if (const AMAGameState* GS = GetGameState<AMAGameState>())
	{
		return GS->GetLoopReadyForPlayer(PlayerState);
	}
	return false;
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
