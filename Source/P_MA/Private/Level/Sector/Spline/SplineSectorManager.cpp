// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineSectorManager.h"
#include "DrawDebugHelpers.h"
#include "Framework/MAGameMode.h"

#include "Kismet/GameplayStatics.h"


ASplineSectorManager::ASplineSectorManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASplineSectorManager::BeginPlay()
{
	Super::BeginPlay();

	PlatformRoot = Cast<APlatformRoot>(
	UGameplayStatics::GetActorOfClass(GetWorld(), APlatformRoot::StaticClass())
	);

	CachingMAGameMode();
	SetSplinesWithMAGameState(CachedMAGameMode->GetMAGameState());
}

bool ASplineSectorManager::IsClosePreSectorZeroVector()
{
	if (!PlatformRoot) return false;

	FVector PlatformLoc = PlatformRoot->GetActorLocation();
	PlatformLoc.Y = 0; PlatformLoc.Z = 0;
	FVector FinalSectorLoc = Sectors[Sectors.Num() - 1]->GetActorLocation();
	FinalSectorLoc.Y = 0; FinalSectorLoc.Z = 0;
	
	float CenterDistance = (PlatformLoc - FinalSectorLoc).Length();

	UE_LOG(LogTemp, Display, TEXT("CenterDistance: %f"), CenterDistance);
	DrawDebugSphere(GetWorld(), FinalSectorLoc, 12.f, 12.f,FColor::Red, true, 1, 0, 0);
	
	if (CenterDistance < 100.f)
		return true;
	return false;
}

void ASplineSectorManager::GoBackToFirstSector()
{
	int32 LastSectorIndex = Sectors.Num() - 1;
	UE_LOG(LogTemp, Display, TEXT("LastSectorIndex: %d"), LastSectorIndex);

	Sectors[0]->SetSectorSeed(Sectors[LastSectorIndex]->GetSectorSeed());
	
	FVector FirstSectorLoc = Sectors[0]->GetActorLocation();
	FVector LastSectorLoc = Sectors[LastSectorIndex]->GetActorLocation();
	FVector PlatformLoc = PlatformRoot->GetActorLocation();

	FVector Offset = PlatformLoc - LastSectorLoc;
	FirstSectorLoc += Offset;
	
	FirstSectorLoc.Z = 50;
	PlatformRoot->SetActorLocation(FirstSectorLoc);
}

int32 ASplineSectorManager::GetNextSectorIndex(int32 CurSectorIndex)
{
	int32 LastSectorIndex = Sectors.Num() - 1;
	return CurSectorIndex == LastSectorIndex ? 0 : CurSectorIndex + 1;
}

ASplineSectorManager* ASplineSectorManager::FindSplineSectorManager(UWorld* World)
{
	AActor* Found = UGameplayStatics::GetActorOfClass(World, ASplineSectorManager::StaticClass());
	ASplineSectorManager* SSM = Cast<ASplineSectorManager>(Found);
	return SSM;
}

void ASplineSectorManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		const FString PrevName = EnumPtr->GetNameStringByValue((int64)CachedPrevMAGameState);
		const FString CurrName = EnumPtr->GetNameStringByValue((int64)InMAGS);
		UE_LOG(LogTemp, Display, TEXT("PrevState: %s"), *PrevName);
		UE_LOG(LogTemp, Display, TEXT("CurrState: %s"), *CurrName);
		UE_LOG(LogTemp, Display, TEXT("- - - - -"));
	}
	
	if (InMAGS == EMAGameState::Wait)
	{
		Sectors.Empty();
		bIsMoving = false;
	}
	else if (InMAGS == EMAGameState::Start)
	{
		if (CachedPrevMAGameState == EMAGameState::Start)
		{
			CachedPrevMAGameState = InMAGS;
			SetSplinesWithMAGameState(EMAGameState::InBattle);
			CachedMAGameMode->SetMAGameState(EMAGameState::InBattle);
			return;
		}
		else
		{
			Sectors = StartSectors;
			bIsMoving = true;
		}
	}
	else if (InMAGS == EMAGameState::InBattle)
	{
		if (CachedPrevMAGameState == EMAGameState::InBattle)
		{
			CachedPrevMAGameState = InMAGS;
			SetSplinesWithMAGameState(EMAGameState::Battle);
			CachedMAGameMode->SetMAGameState(EMAGameState::Battle);
			CachedMAGameMode->StartWave();
			return;
		}
		else
		{
			Sectors = InBattleSectors;
			bIsMoving = true;
		}
	}
	else if (InMAGS == EMAGameState::Battle)
	{
		Sectors.Empty();
		bIsMoving = false;
		CachedMAGameMode->EndWave();
	}
	else if (InMAGS == EMAGameState::EndBattle)
	{
		Sectors.Empty();
		bIsMoving = false;
	}
	else if (InMAGS == EMAGameState::OutBattle)
	{
		if (CachedPrevMAGameState == EMAGameState::OutBattle)
		{
			CachedPrevMAGameState = InMAGS;
			SetSplinesWithMAGameState(EMAGameState::Loop);
			CachedMAGameMode->SetMAGameState(EMAGameState::Loop);
			return;
		}
		else
		{
			Sectors = OutBattleSectors;
			bIsMoving = true;
		}
	}
	else if (InMAGS == EMAGameState::Loop)
	{
		Sectors = LoopSectors;
		bIsMoving = true;
	}

	CachedPrevMAGameState = InMAGS;
}
