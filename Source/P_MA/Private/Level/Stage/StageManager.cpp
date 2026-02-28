// Fill out your copyright notice in the Description page of Project Settings.

#include "Level/Stage/StageManager.h"
#include "Framework/MAGameMode.h"
#include "Framework/MAGameState.h"
#include "Kismet/GameplayStatics.h"

AStageManager::AStageManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AStageManager::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	CachedMAGameMode = Cast<AMAGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (CachedMAGameMode)
	{
		CachedMAGameMode->OnMASectorStateChanged.AddUObject(this, &AStageManager::OnHandleSectorStateChanged);
		CachedMASectorState = CachedMAGameMode->GetMASectorState();
		OnHandleSectorStateChanged(CachedMASectorState);
		if (AMAGameState* GS = CachedMAGameMode->GetGameState<AMAGameState>())
		{
			GS->SetStageCycle(CurStageCycleData);
		}
	}
}

void AStageManager::OnHandleSectorStateChanged(EMASectorState NewState)
{
	if (!HasAuthority()) return;

	if (NewState == EMASectorState::Loop && CachedMASectorState != EMASectorState::Loop)
	{
		AdvanceStage();
	}

	CachedMASectorState = NewState;
}

void AStageManager::AdvanceStage()
{
	const int32 MaxStageCount = GetMaxStageCount();
	if (MaxStageCount <= 0) return;

	CurStageCycleData.Stage++;
	if (CurStageCycleData.Stage > MaxStageCount)
	{
		CurStageCycleData.Stage = 1;
		CurStageCycleData.Round++;
	}

	UE_LOG(LogTemp, Warning, TEXT("StageManager: AdvanceStage -> %d-%d (Max=%d)"),
		CurStageCycleData.Round, CurStageCycleData.Stage, MaxStageCount);

	if (CachedMAGameMode)
	{
		if (AMAGameState* GS = CachedMAGameMode->GetGameState<AMAGameState>())
		{
			GS->SetStageCycle(CurStageCycleData);
		}
	}

	const FStageSetting& NewStageSetting = GetCurrentStageSetting();
	if (NewStageSetting.bChangeEnv)
	{
		OnStageChangeEnvRequested.Broadcast();
	}
}

const FStageSetting& AStageManager::GetCurrentStageSetting() const
{
	if (StageSettings.Num() == 0)
	{
		static const FStageSetting Dummy;
		return Dummy;
	}

	const int32 StageCount = StageSettings.Num();
	const int32 Index = FMath::Clamp(CurStageCycleData.Stage - 1, 0, StageCount - 1);
	return StageSettings[Index];
}
