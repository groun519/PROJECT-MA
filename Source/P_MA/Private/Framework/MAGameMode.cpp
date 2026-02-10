// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MAGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerState.h"
#include "Player/ReadyStateComponent.h"

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

void AMAGameMode::RequestStateChange(EMAGameState NewState)
{
	if (MAGameState == NewState) return;

	MAGameState = NewState;

	if (OnMAGameStateChanged.IsBound())
	{
		OnMAGameStateChanged.Broadcast(MAGameState);
	}
}

EMAGameState AMAGameMode::GetNextState(EMAGameState CurState) const
{
	if (CurState == EMAGameState::Loop)
	{
		return EMAGameState::Start;
	}

	return static_cast<EMAGameState>(static_cast<int32>(CurState) + 1);
}

void AMAGameMode::RequestNextState(EMAGameState CurState)
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
			RequestNextState(MAGameState);
		}
	}

	if (OnReadyCountChanged.IsBound())
	{
		OnReadyCountChanged.Broadcast(ReadyCount, TotalCount);
	}
}

void AMAGameMode::SetMAState(int32 NewState)
{
	if (NewState < 0 || NewState > static_cast<int32>(EMAGameState::Loop))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetMAState: invalid state %d"), NewState);
		return;
	}

	RequestStateChange(static_cast<EMAGameState>(NewState));
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
