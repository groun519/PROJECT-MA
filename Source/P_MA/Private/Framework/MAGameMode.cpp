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
		TSubclassOf<AMonster> Monster; int32 Cost = 0;
		GetRandomMonsterByEnv(Monster, Cost, CurEnvTag);

		if (Cost == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Monster Cost is Zero !"));
			break;
		}
		if (UsingCost + Cost > TotalWaveCost) continue;

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
	if (bIsWaving) return;
	
	bIsWaving = true;
	WaveMonsters = GetNewWaveMonsters();
	CreateBaseIntervalTimer();
}

void AMAGameMode::EndWave()
{
	if (!bIsWaving) return;
	
	bIsWaving = false;
	if (Wave == 5)
	{
		Stage++;
		Wave = 1;
	}
	else
	{
		Wave++;
	}
}

void AMAGameMode::SpawnMonsters(int32 SpawnAtOnce)
{
	if (WaveMonsters.IsEmpty()) return;
	
	TArray<FVector> SpawnLocations
		= SpawnSpline->GetMonsterSpawnLocations(SpawnAtOnce);

	for (FVector SpawnLoc : SpawnLocations)
	{
		if (WaveMonsters.Num() == 0) return;
		FWaveMonster Monster = WaveMonsters[0];
		if (!Monster.Class) continue;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FVector SpawnLocation = SpawnSpline->GetActorLocation() + SpawnLoc;
		FRotator SpawnRotation = (-SpawnLoc).Rotation();
		
		AMonster* Spawned = GetWorld()->SpawnActor<AMonster>(
			Monster.Class,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);
		Spawned->SetGoal(SpawnSpline);

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