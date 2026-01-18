// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineSectorManager.h"
#include "DrawDebugHelpers.h"
#include "Framework/MAGameMode.h"
#include "Kismet/GameplayStatics.h"

ASplineSectorManager::ASplineSectorManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASplineSectorManager::BeginPlay()
{
	Super::BeginPlay();

	PlatformRoot = Cast<APlatformRoot>(
	UGameplayStatics::GetActorOfClass(GetWorld(), APlatformRoot::StaticClass())
	);

	if (HasAuthority())
	{
		/** GameMode **/
		AMAGameMode* MAGM = Cast<AMAGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (MAGM)
		{
			CachedMAGameMode = MAGM;
			CachedMAGameMode->OnMAGameStateChanged.AddUObject(this, &ASplineSectorManager::OnHandleGameStateChanged);
			OnHandleGameStateChanged(CachedMAGameMode->GetMAGameState());
			CachedMAGameMode->OnAllPlayersReady.AddUObject(this, &ASplineSectorManager::OnHandleAllPlayersReady);
		}
		
		/** PlatformRoot **/
		APlatformRoot* PR = Cast<APlatformRoot>(UGameplayStatics::GetActorOfClass(GetWorld(), APlatformRoot::StaticClass()));
		if (PR)
		{
			CachedPlatformRoot = PR;
			CachedPlatformRoot->OnPlatformReachedEnd.AddUObject(this, &ASplineSectorManager::OnHandlePlatformReachedEnd);
			if (CachedMAGameMode)
			{
				bool bWaitMoveIn =
					CachedMAGameState == EMAGameState::Wait || CachedMAGameState == EMAGameState::EndBattle;
				CachedPlatformRoot->SetWaitMoveIn(bWaitMoveIn);
				CachedPlatformRoot->SetHeight(bIsMoving);
			}
		}
	}
}

void ASplineSectorManager::OnHandleGameStateChanged(EMAGameState NewState)
{
	LogStateChange(NewState);
	bool bWasMoving = bIsMoving;
	SetSectorsByState(NewState);
	CachedMAGameState = NewState;
	CurSectorIndex = 0;
	
	if (CachedPlatformRoot)
	{
		bool bWaitMoveIn =
			NewState == EMAGameState::Wait || NewState == EMAGameState::EndBattle;
		CachedPlatformRoot->SetWaitMoveIn(bWaitMoveIn);
		CachedPlatformRoot->SetHeight(bIsMoving);
	}

	if (!bIsMoving && CachedPlatformRoot)
	{
		CachedPlatformRoot->SetCurSpline(nullptr);
	}

	if (!bWasMoving && bIsMoving)
	{
		ApplySplineSelection();
	}
	UE_LOG(LogTemp, Warning, TEXT("SplineManager: 상태 변화 감지 -> %d"), (int32)NewState);
}

void ASplineSectorManager::OnHandlePlatformReachedEnd()
{
	if (CurSectors.IsEmpty() || !CachedPlatformRoot) return;
	
	bool bIsLastSector = CurSectorIndex >= CurSectors.Num() - 1;
	if (bIsLastSector)
	{
		CurSectorIndex = 0;
	}
	else
	{
		CurSectorIndex++;
	}

	bool bRequestedStateChange = false;
	if (bIsLastSector)
	{
		LogStateChange(CachedMAGameState);
		bRequestedStateChange = HandleRepeatState(CachedMAGameState);
	}

	if (bRequestedStateChange)
	{
		ApplySplineSelection();
		return;
	}

	ApplySplineSelection();
	
	UE_LOG(LogTemp, Warning, TEXT("SplineManager: 플랫폼 섹터 끝 도달!"));
}

void ASplineSectorManager::OnHandleAllPlayersReady()
{
}

int32 ASplineSectorManager::GetNextSectorIndex(int32 InSectorIndex)
{
	int32 LastSectorIndex = CurSectors.Num() - 1;
	if (InSectorIndex == LastSectorIndex)
	{
		return 0;
	}
	else
	{
		return InSectorIndex + 1;
	}
}

ASplineSectorManager* ASplineSectorManager::FindSplineSectorManager(UWorld* World)
{
	AActor* Found = UGameplayStatics::GetActorOfClass(World, ASplineSectorManager::StaticClass());
	ASplineSectorManager* SSM = Cast<ASplineSectorManager>(Found);
	return SSM;
}

void ASplineSectorManager::GoToNextState(EMAGameState InNextState)
{
	if (!CachedMAGameMode) return;
	
	/** WaveManager 만들고 나면 옮길 파트 **/
	if (InNextState == EMAGameState::Battle)
	{
		CachedMAGameMode->StartWave();
	}
	else if (InNextState == EMAGameState::EndBattle && CachedMAGameMode->bIsWaving)
	{
		CachedMAGameMode->EndWave();
	}
	/****/

	// 리퀘스트 보냄
	CachedMAGameMode->RequestStateChange(InNextState);
}

void ASplineSectorManager::SetSectorsByState(EMAGameState InState)
{
	FSplineSectorData SSData = SplineSectorsByState[InState];
	
	bIsMoving = SSData.bIsMoving;
	
	if (bIsMoving)
		CurSectors = SSData.Sectors;
	else
		CurSectors.Empty();
}

bool ASplineSectorManager::HandleRepeatState(EMAGameState InState)
{
	if (InState == EMAGameState::Start)
	{
		GoToNextState(EMAGameState::InBattle);
		return true;
	}
	if (InState == EMAGameState::InBattle)
	{
		GoToNextState(EMAGameState::Battle);
		return true;
	}
	if (InState == EMAGameState::OutBattle)
	{
		GoToNextState(EMAGameState::Loop);
		return true;
	}

	return false;
}

void ASplineSectorManager::ApplySplineSelection()
{
	if (!CachedPlatformRoot) return;

	if (!bIsMoving || CurSectors.IsEmpty())
	{
		CachedPlatformRoot->SetCurSpline(nullptr);
		return;
	}

	if (CurSectorIndex < 0 || CurSectorIndex >= CurSectors.Num())
	{
		CurSectorIndex = 0;
	}

	if (!CurSectors[CurSectorIndex])
	{
		CachedPlatformRoot->SetCurSpline(nullptr);
		return;
	}

	int32 NextIndex = GetNextSectorIndex(CurSectorIndex);
	CurSectors[NextIndex]->SetRandomSeed();

	USplineComponent* CurSpline = CurSectors[CurSectorIndex]->RoadSpline;
	if (!IsValid(CurSpline))
	{
		CachedPlatformRoot->SetCurSpline(nullptr);
		return;
	}
	CachedPlatformRoot->SetCurSpline(CurSpline);
}

void ASplineSectorManager::LogStateChange(EMAGameState InState) const
{
	if (!bUseStateDebug) return;

	const UEnum* EnumPtr = StaticEnum<EMAGameState>();
	const FString PrevName = EnumPtr->GetNameStringByValue((int64)CachedMAGameState);
	const FString CurrName = EnumPtr->GetNameStringByValue((int64)InState);
	UE_LOG(LogTemp, Display, TEXT("PrevState: %s"), *PrevName);
	UE_LOG(LogTemp, Display, TEXT("CurrState: %s"), *CurrName);
	UE_LOG(LogTemp, Display, TEXT("- - - - -"));
}
