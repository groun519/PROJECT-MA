#include "WaveManager.h"

#include "AI/Monster/Monster.h"
#include "Framework/MAGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "AI/Data/MonstersByEnvironmentData.h"
#include "AbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "Level/Environment/EnvironmentManager.h"

AWaveManager::AWaveManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWaveManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && !InitCachedMAGameMode())
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveManager: MAGameMode not Found"));
	}
	if (!InitSpawnSpline())
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveManager: SpawnSpline not Found"));
	}
	if (HasAuthority() && !BindEnvironmentManager())
	{
		UE_LOG(LogTemp, Warning, TEXT("WaveManager: EnvironmentManager not Found"));
	}
}

void AWaveManager::OnHandleSectorStateChanged(EMASectorState NewState)
{
	if (!HasAuthority()) return;

	if (NewState == EMASectorState::Battle)
	{
		StartWave();
	}
	else if (NewState == EMASectorState::EndBattle)
	{
		EndWave();
	}
}

TArray<FWaveMonster> AWaveManager::GetNewWaveMonsters()
{
	SetTotalCoinByWave();
	SetStatCoefficientByWave();
	int32 UsingCoin = 0;
	TArray<FWaveMonster> OutWaveMonsters;

	int32 MinCoin = 0;
	if (MonsByEnvData)
	{
		FString TagString = CurEnvTag.ToString();
		FString Last;
		TagString.Split(TEXT("."), nullptr, &Last, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		FName RowName(*Last);

		FMonstersByEnvironmentData* Data = MonsByEnvData->FindRow<FMonstersByEnvironmentData>(
			RowName,
			TEXT("GetNewWaveMonsters"),
			false
		);
		if (Data && CurEnvTag == Data->EnvGameplayTag)
		{
			for (const auto& Pair : Data->MonsterToCoin)
			{
				if (Pair.Value <= 0) continue;
				MinCoin = (MinCoin == 0) ? Pair.Value : FMath::Min(MinCoin, Pair.Value);
			}
		}
	}
	
	if (MinCoin == 0 || TotalCoin <= 0)
	{
		return OutWaveMonsters;
	}

	while (UsingCoin + MinCoin <= TotalCoin && OutWaveMonsters.Num() < WaveSetting.MaxMonsterNum)
	{
		TSubclassOf<AMonster> Monster;
		int32 SpawnCostCoin = 0;
		GetRandomMonsterByEnv(Monster, SpawnCostCoin, CurEnvTag, TotalCoin - UsingCoin);

		if (!Monster || SpawnCostCoin <= 0) break;

		FWaveMonster NewMonster{Monster, SpawnCostCoin};
		OutWaveMonsters.Add(NewMonster);
		UsingCoin += SpawnCostCoin;
	}

	LastCoin = FMath::Max(0, TotalCoin - UsingCoin);
	if (TotalCoin > 0 && LastCoin > 0)
	{
		const float RemainRatio = static_cast<float>(LastCoin) / static_cast<float>(TotalCoin);
		MonsterStatCoefficient *= (1.0f + RemainRatio);
	}

	OutWaveMonsters.Sort([](const FWaveMonster& A, const FWaveMonster& B)
	{
		return A.SpawnCostCoin < B.SpawnCostCoin;
	});
	
	return OutWaveMonsters;
}

void AWaveManager::GetRandomMonsterByEnv(
	TSubclassOf<AMonster>& OutMonster,
	int32& OutSpawnCostCoin,
	FGameplayTag InEnvTag,
	int32 MaxSpawnCostCoin)
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
	
	TArray<TSubclassOf<AMonster>> MonsterClasses;
	TArray<int32> SpawnCostCoins;
	for (const auto& Pair : Data->MonsterToCoin)
	{
		if (!Pair.Key || Pair.Value <= 0) continue;
		if (MaxSpawnCostCoin > 0 && Pair.Value > MaxSpawnCostCoin) continue;

		MonsterClasses.Add(Pair.Key);
		SpawnCostCoins.Add(Pair.Value);
	}

	if (MonsterClasses.IsEmpty()) return;
	
	int32 RandomIndex = FMath::RandRange(0, MonsterClasses.Num() - 1);
	
	OutMonster = MonsterClasses[RandomIndex];
	OutSpawnCostCoin = SpawnCostCoins[RandomIndex];
}

void AWaveManager::StartWave()
{
	if (bIsWaving) return;
	
	bIsWaving = true;
	bWaveSpawnFinished = false;
	AliveMonsterCount = 0;
	WaveMonsters = GetNewWaveMonsters();
	CreateBaseIntervalTimer();
}

void AWaveManager::EndWave()
{
	if (!bIsWaving) return;
	
	bIsWaving = false;
	bWaveSpawnFinished = false;
	GetWorldTimerManager().ClearTimer(BaseIntervalTimerHandle);

	Wave++;
}

int32 AWaveManager::SpawnMonstersAndReturnCoin(int32 SpawnAtOnce)
{
	if (WaveMonsters.IsEmpty() || !SpawnSpline) return 0;
	
	TArray<FVector> SpawnLocations
		= SpawnSpline->GetMonsterSpawnLocations(SpawnAtOnce);

	int32 UsingCoin = 0;
	
	for (FVector SpawnLoc : SpawnLocations)
	{
		if (WaveMonsters.Num() == 0) break;

		// 첫 인덱스의 몬스터 픽
		FWaveMonster Monster = WaveMonsters[0];
		WaveMonsters.RemoveAt(0);
		if (!Monster.Class) continue;

		// 코인 +
		UsingCoin += Monster.SpawnCostCoin;

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FVector SpawnLocation = SpawnSpline->GetActorLocation() + SpawnLoc;
		FRotator SpawnRotation = (-SpawnLoc).Rotation();
		
		const FTransform SpawnTransform(SpawnRotation, SpawnLocation);
		AMonster* Spawned = GetWorld()->SpawnActorDeferred<AMonster>(
			Monster.Class,
			SpawnTransform,
			nullptr,
			nullptr,
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		);
		if (Spawned)
		{
			Spawned->SetEnvTag(CurEnvTag);
			Spawned->SetStatCoefficient(MonsterStatCoefficient);
			Spawned->FinishSpawning(SpawnTransform);
			Spawned->GetAbilitySystemComponent()->SetNumericAttributeBase(UMAAttributeSet::GetCoinAttribute(), Monster.SpawnCostCoin);
			Spawned->SetGoal(SpawnSpline);
			Spawned->OnMonsterDead.AddUObject(this, &AWaveManager::OnMonsterDead);
			AliveMonsterCount++;
		}

	}

	return UsingCoin;
}

void AWaveManager::CreateBaseIntervalTimer()
{
	LastCoin = TotalCoin;

	if (!GetWorld()) return;

	GetWorldTimerManager().SetTimer(
		BaseIntervalTimerHandle,
		this,
		&AWaveManager::SpawnMonstersByInterval,
		SpawnInterval,
		true 
	);
}

void AWaveManager::SpawnMonstersByInterval()
{
	if (!bIsWaving) return;

	int32 UsingCoin =
		SpawnMonstersAndReturnCoin(1);

	LastCoin -= UsingCoin;
	
	if (WaveMonsters.IsEmpty())
	{
		bWaveSpawnFinished = true;
		GetWorldTimerManager().ClearTimer(BaseIntervalTimerHandle);
		TryEndWave();
	}
}

void AWaveManager::OnMonsterDead()
{
	if (AliveMonsterCount > 0)
	{
		AliveMonsterCount--;
	}
	TryEndWave();
}

void AWaveManager::TryEndWave()
{
	if (!bIsWaving) return;
	if (!bWaveSpawnFinished) return;
	if (AliveMonsterCount > 0) return;

	if (CachedMAGameMode)
	{
		CachedMAGameMode->RequestStateChange(EMASectorState::EndBattle);
	}
}

/** Init Helper **/
bool AWaveManager::InitCachedMAGameMode()
{
	CachedMAGameMode = Cast<AMAGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (CachedMAGameMode)
	{
		CachedMAGameMode->OnMASectorStateChanged.AddUObject(this, &AWaveManager::OnHandleSectorStateChanged);
		OnHandleSectorStateChanged(CachedMAGameMode->GetMASectorState());
		return true;
	}
	return false;
}

bool AWaveManager::InitSpawnSpline()
{
	if (SpawnSpline) return true;
	if (GetWorld())
	{
		TArray<AActor*> FoundSplines;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABattleSpaceSpline::StaticClass(), FoundSplines);

		const FName BattleSpaceTag(TEXT("BattleSpace"));
		for (AActor* Actor : FoundSplines)
		{
			if (Actor && Actor->ActorHasTag(BattleSpaceTag))
			{
				SpawnSpline = Cast<ABattleSpaceSpline>(Actor);
				break;
			}
		}

		if (!SpawnSpline)
		{
			SpawnSpline = Cast<ABattleSpaceSpline>(
				UGameplayStatics::GetActorOfClass(GetWorld(), ABattleSpaceSpline::StaticClass())
			);
			return false;
		}
		return true;
	}
	return false;
}

bool AWaveManager::BindEnvironmentManager()
{
	AEnvironmentManager* EnvironmentManager = AEnvironmentManager::FindEnvironmentManager(GetWorld());
	if (!EnvironmentManager) return false;

	EnvironmentManager->OnEnvironmentTagChanged.AddUObject(this, &AWaveManager::OnEnvironmentChanged);
	EnvironmentManager->BroadcastCurrentEnvironment();
	return true;
}

void AWaveManager::OnEnvironmentChanged(const FGameplayTag& NewEnvTag)
{
	CurEnvTag = NewEnvTag;
}
