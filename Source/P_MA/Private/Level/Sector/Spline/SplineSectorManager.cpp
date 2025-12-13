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
		CachingMAGameMode();
		SetSplinesWithMAGameState(CachedMAGameMode->GetMAGameState());
	}
}

int32 ASplineSectorManager::GetNextSectorIndex(int32 CurSectorIndex)
{
	int32 LastSectorIndex = CurSectors.Num() - 1;
	return CurSectorIndex == LastSectorIndex ? 0 : CurSectorIndex + 1;
}

ASplineSectorManager* ASplineSectorManager::FindSplineSectorManager(UWorld* World)
{
	AActor* Found = UGameplayStatics::GetActorOfClass(World, ASplineSectorManager::StaticClass());
	ASplineSectorManager* SSM = Cast<ASplineSectorManager>(Found);
	return SSM;
}

void ASplineSectorManager::CachingMAGameMode()
{
	AMAGameMode* MAGM = Cast<AMAGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (MAGM) CachedMAGameMode = MAGM;
}

void ASplineSectorManager::SetSplinesWithMAGameState(EMAGameState InMAGS)
{
	if (bUseStateDebug)
	{
		const UEnum* EnumPtr = StaticEnum<EMAGameState>();
		const FString PrevName = EnumPtr->GetNameStringByValue((int64)CachedMAGameState);
		const FString CurrName = EnumPtr->GetNameStringByValue((int64)InMAGS);
		UE_LOG(LogTemp, Display, TEXT("PrevState: %s"), *PrevName);
		UE_LOG(LogTemp, Display, TEXT("CurrState: %s"), *CurrName);
		UE_LOG(LogTemp, Display, TEXT("- - - - -"));
	}
	
	if (InMAGS == EMAGameState::Wait)
	{
		if (SameAsCachedState(InMAGS))
			return;
	}
	else if (InMAGS == EMAGameState::Start)
	{
		if (SameAsCachedState(InMAGS))
		{
			GoToNextState(InMAGS, EMAGameState::InBattle);
			return;
		}
	}
	else if (InMAGS == EMAGameState::InBattle)
	{
		if (SameAsCachedState(InMAGS))
		{
			GoToNextState(InMAGS, EMAGameState::Battle);
			CachedMAGameMode->StartWave();
			return;
		}
	}
	else if (InMAGS == EMAGameState::Battle)
	{
		if (SameAsCachedState(InMAGS))
			return;
	}
	else if (InMAGS == EMAGameState::EndBattle)
	{
		if (SameAsCachedState(InMAGS))
			return;
		
		if (CachedMAGameMode->bIsWaving)
		{
			CachedMAGameMode->EndWave();
		}
	}
	else if (InMAGS == EMAGameState::OutBattle)
	{
		if (SameAsCachedState(InMAGS))
		{
			GoToNextState(InMAGS, EMAGameState::Loop);
			return;
		}
	}
	else if (InMAGS == EMAGameState::Loop)
	{
		if (SameAsCachedState(InMAGS))
			return;
	}
	SetSectorsByState(InMAGS);
	CachedMAGameState = InMAGS;
}

void ASplineSectorManager::GoToNextState(EMAGameState InCurState,EMAGameState InNextState)
{
	CachedMAGameState = InCurState;
	SetSplinesWithMAGameState(InNextState);
	CachedMAGameMode->SetMAGameState(InNextState);
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
