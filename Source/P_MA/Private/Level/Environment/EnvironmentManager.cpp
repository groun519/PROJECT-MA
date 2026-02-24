// Fill out your copyright notice in the Description page of Project Settings.

#include "Level/Environment/EnvironmentManager.h"
#include "AI/Data/MonstersByEnvironmentData.h"
#include "Framework/MAGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "Level/Stage/StageManager.h"
#include "TimerManager.h"

AEnvironmentManager::AEnvironmentManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AEnvironmentManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		InitCachedMAGameMode();
		InitStageManager();

		// Initialize current env immediately.
		if (!CurrentEnvTag.IsValid())
		{
			FGameplayTag InitialEnvTag;
			if (PickRandomDifferentEnvTag(InitialEnvTag))
			{
				CurrentEnvTag = InitialEnvTag;
			}
		}

		// Push initial snapshot on next tick so listeners that bind in BeginPlay don't miss it.
		GetWorldTimerManager().SetTimerForNextTick(this, &AEnvironmentManager::BroadcastCurrentEnvironment);
	}
}

AEnvironmentManager* AEnvironmentManager::FindEnvironmentManager(UWorld* InWorld)
{
	if (!InWorld) return nullptr;
	return Cast<AEnvironmentManager>(UGameplayStatics::GetActorOfClass(InWorld, AEnvironmentManager::StaticClass()));
}

bool AEnvironmentManager::SetCurrentEnvTag(FGameplayTag InEnvTag)
{
	if (!HasAuthority()) return false;
	if (!InEnvTag.IsValid()) return false;
	if (CurrentEnvTag == InEnvTag) return false;

	PreviousEnvTag = CurrentEnvTag;
	CurrentEnvTag = InEnvTag;
	OnEnvironmentTagChanged.Broadcast(CurrentEnvTag);
	OnEnvironmentPCGChanged.Broadcast(FindPCGGraphByTag(CurrentEnvTag));
	return true;
}

void AEnvironmentManager::BroadcastCurrentEnvironment()
{
	OnEnvironmentTagChanged.Broadcast(CurrentEnvTag);
	OnEnvironmentPCGChanged.Broadcast(FindPCGGraphByTag(CurrentEnvTag));
}

bool AEnvironmentManager::InitCachedMAGameMode()
{
	CachedMAGameMode = Cast<AMAGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (!CachedMAGameMode) return false;

	CachedMAGameMode->OnMASectorStateChanged.AddUObject(this, &AEnvironmentManager::OnHandleSectorStateChanged);
	OnHandleSectorStateChanged(CachedMAGameMode->GetMASectorState());
	return true;
}

bool AEnvironmentManager::InitStageManager()
{
	AStageManager* StageManager = Cast<AStageManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AStageManager::StaticClass()));
	if (!StageManager) return false;

	StageManager->OnStageChangeEnvRequested.AddUObject(this, &AEnvironmentManager::OnHandleStageChangeEnvRequested);
	return true;
}

void AEnvironmentManager::OnHandleSectorStateChanged(EMASectorState NewState)
{
	CachedMASectorState = NewState;

	if (CachedMASectorState == EMASectorState::OutBattle)
	{
		BroadcastCurrentEnvironment();
	}
}

void AEnvironmentManager::OnHandleStageChangeEnvRequested()
{
	if (!HasAuthority()) return;

	FGameplayTag NewEnvTag;
	if (!PickRandomDifferentEnvTag(NewEnvTag)) return;

	SetCurrentEnvTag(NewEnvTag);
}

UPCGGraph* AEnvironmentManager::FindPCGGraphByTag(FGameplayTag InEnvTag) const
{
	if (!EnvironmentDataTable || !InEnvTag.IsValid()) return nullptr;

	TArray<FMonstersByEnvironmentData*> Rows;
	EnvironmentDataTable->GetAllRows(TEXT("FindPCGGraphByTag"), Rows);

	for (const FMonstersByEnvironmentData* Row : Rows)
	{
		if (!Row) continue;
		if (Row->EnvGameplayTag != InEnvTag) continue;
		return Row->EnvPCGGraph;
	}

	return nullptr;
}

bool AEnvironmentManager::PickRandomDifferentEnvTag(FGameplayTag& OutEnvTag) const
{
	if (!EnvironmentDataTable) return false;

	TArray<FMonstersByEnvironmentData*> Rows;
	EnvironmentDataTable->GetAllRows(TEXT("PickRandomDifferentEnvTag"), Rows);

	TArray<FGameplayTag> Candidates;
	for (const FMonstersByEnvironmentData* Row : Rows)
	{
		if (!Row) continue;
		if (!Row->EnvGameplayTag.IsValid()) continue;
		if (Row->EnvGameplayTag == CurrentEnvTag) continue;
		Candidates.AddUnique(Row->EnvGameplayTag);
	}

	if (Candidates.IsEmpty()) return false;

	// On runtime env changes, prefer not to bounce back to the immediate previous env.
	// On initial startup (CurrentEnvTag invalid), keep full random among all candidates.
	if (CurrentEnvTag.IsValid() && PreviousEnvTag.IsValid())
	{
		TArray<FGameplayTag> Preferred;
		for (const FGameplayTag& Candidate : Candidates)
		{
			if (Candidate == PreviousEnvTag) continue;
			Preferred.Add(Candidate);
		}

		if (!Preferred.IsEmpty())
		{
			const int32 RandomIndex = FMath::RandRange(0, Preferred.Num() - 1);
			OutEnvTag = Preferred[RandomIndex];
			return true;
		}
	}

	const int32 RandomIndex = FMath::RandRange(0, Candidates.Num() - 1);
	OutEnvTag = Candidates[RandomIndex];
	return true;
}
