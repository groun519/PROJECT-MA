// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MAGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "AI/Data/MonstersByEnvironmentData.h"
#include "Kismet/GameplayStatics.h"

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

	if (GetWorld())
	{
		ABattleSpaceSpline* Found = Cast<ABattleSpaceSpline>(
			UGameplayStatics::GetActorOfClass(GetWorld(), ABattleSpaceSpline::StaticClass())
		);

		SpawnSpline = Found;
	}

	StartWave();
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

TArray<FWaveMonster> AMAGameMode::GetNewWaveMonsters()
{
	SetTotalWaveCost();
	int32 UsingCost = 0;
	TArray<FWaveMonster> OutWaveMonsters;
	
	while (UsingCost != TotalWaveCost)
	{
		TSubclassOf<AMonster> Monster; int32 Cost;
		GetRandomMonsterByEnv(Monster, Cost, EnvTag);

		if (Cost + UsingCost > TotalWaveCost) continue;

		FWaveMonster NewMonster(Monster, Cost);
		OutWaveMonsters.Add(NewMonster);
		UsingCost += Cost;
	}

	OutWaveMonsters.Sort([](const FWaveMonster& A, const FWaveMonster& B)
	{
		return A.Cost < B.Cost;   
	});
	
	return OutWaveMonsters;
}

void AMAGameMode::GetRandomMonsterByEnv(TSubclassOf<AMonster>& OutMonster, int32& OutCost, FGameplayTag InEnvTag)
{
	if (!MonsByEnvData) return;
	
	FString TagString = InEnvTag.ToString();
	FString Last;
	TagString.Split(TEXT("."), nullptr, &Last, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	FName RowName(*Last);

	FMonstersByEnvironmentData* Data = MonsByEnvData->FindRow<FMonstersByEnvironmentData>(
		RowName,
		TEXT("GetRandomMonsterByEnv"),
		false
	);
	if (!Data) return;
	if (InEnvTag != Data->EnvGameplayTag) return;
	
	TArray<TSubclassOf<AMonster>> Keys;
	Data->MonsterData.GetKeys(Keys);

	if (Keys.Num() == 0) return;
	
	int32 RandomIndex = FMath::RandRange(0, Data->MonsterData.Num() - 1);
	
	OutMonster = Keys[RandomIndex];
	OutCost = Data->MonsterData[OutMonster];
}

void AMAGameMode::StartWave()
{
	WaveMonsters = GetNewWaveMonsters();
	CreateBaseIntervalTimer();
}

void AMAGameMode::EndWave()
{
	if (Wave == 5)
	{
		Stage++;
	}
	else
	{
		Wave++;
	}
}

void AMAGameMode::SpawnMonsters(int32 SpawnAtOnce)
{
	if (WaveMonsters.IsEmpty()) return;
	
	int32 Count = 0;
	
	for (FWaveMonster Monster : WaveMonsters)
	{
		if (!Monster.Class) continue;
		if (SpawnAtOnce <= Count) return;
		Count++;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		FVector SpawnLocation = FVector::ZeroVector; // 원하는 스폰 위치
		FRotator SpawnRotation = FRotator::ZeroRotator;
		
		AMonster* Spawned = GetWorld()->SpawnActor<AMonster>(
			Monster.Class,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);

		WaveMonsters.RemoveAt(0);
	}
}

void AMAGameMode::CreateBaseIntervalTimer()
{
	LastCostUnit = CostUnit;
	
	int32 IntervalCount = TotalWaveCost / CostUnit + 1;

	int32 IntervalLimit = 20;
	float IntervalSeconds = 1.f;
	while (IntervalCount > IntervalLimit)
	{
		IntervalLimit *= 2;
		IntervalSeconds /= 2;
	}

	if (!GetWorld()) return;

	GetWorldTimerManager().SetTimer(
		BaseIntervalTimerHandle,
		this,
		&AMAGameMode::SpawnMonstersByInterval,
		IntervalSeconds,
		true 
	);
}

void AMAGameMode::SpawnMonstersByInterval()
{
	TArray<TSubclassOf<AMonster>> Monsters;
	int32 Count = 0;
	while (true)
	{
		Count++;
		if (Count > WaveMonsters.Num()) break;

		FWaveMonster Monster = WaveMonsters[Count-1];
		if (LastCostUnit - Monster.Cost <= 0)
		{
			LastCostUnit += CostUnit;
			break;
		}
		else
		{
			LastCostUnit -= Monster.Cost;
		}
	}
	SpawnMonsters(Count);
}

// void AMAGameMode::CreateSchedulizedIntervalTimer(TArray<FVector> InSpawnTargetLoc)
// {
// 	
// }
