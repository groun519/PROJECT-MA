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
		CachedMAGameMode->OnMAGameStateChanged.AddUObject(this, &AStageManager::OnHandleGameStateChanged);
		CachedMAGameState = CachedMAGameMode->GetMAGameState();
		OnHandleGameStateChanged(CachedMAGameState);
		if (AMAGameState* GS = CachedMAGameMode->GetGameState<AMAGameState>())
		{
			GS->SetStageCycle(CurStageCycleData);
		}
	}
}

void AStageManager::OnHandleGameStateChanged(EMAGameState NewState)
{
	if (!HasAuthority()) return;

	if (NewState == EMAGameState::Loop && CachedMAGameState != EMAGameState::Loop)
	{
		AdvanceStage();
	}

	CachedMAGameState = NewState;
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
