// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MAGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Player/MAPlayerCharacter.h"
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
}

void AMAGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
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
