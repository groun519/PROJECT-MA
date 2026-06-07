#include "WaveManager.h"

#include "AI/Golem/Monster.h"
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

	if (!InitCachedMAGameMode())
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
	SetTotalGoldByWave();
	SetStatCoefficientByWave();
	int32 UsingGold = 0;
	TArray<FWaveMonster> OutWaveMonsters;

	int32 MinGold = 0;
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
			for (const auto& Pair : Data->MonsterToData)
			{
				if (Pair.Value.Gold <= 0) continue;
				MinGold = (MinGold == 0) ? Pair.Value.Gold : FMath::Min(MinGold, Pair.Value.Gold);
			}
		}
	}
	
	if (MinGold == 0 || TotalGold <= 0)
	{
		return OutWaveMonsters;
	}

	while (UsingGold + MinGold <= TotalGold && OutWaveMonsters.Num() < WaveSetting.MaxMonsterNum)
	{
		TSubclassOf<AMonster> Monster;
		FMonsterData MonsterData;
		GetRandomMonsterByEnv(Monster, MonsterData, CurEnvTag);

		if (MonsterData.Gold == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("WaveManager: Monster Gold is Zero"));
			break;
		}
		if (UsingGold + MonsterData.Gold > TotalGold)
		{
			UE_LOG(LogTemp, Warning, TEXT("WaveManager: Skip monster gold=%d (UsingGold=%d TotalGold=%d)"),
				MonsterData.Gold, UsingGold, TotalGold);
			continue;
		}

		FWaveMonster NewMonster{Monster, MonsterData};
		OutWaveMonsters.Add(NewMonster);
		UsingGold += MonsterData.Gold;
	}

	LastGold = FMath::Max(0, TotalGold - UsingGold);
	if (TotalGold > 0 && LastGold > 0)
	{
		const float RemainRatio = static_cast<float>(LastGold) / static_cast<float>(TotalGold);
		MonsterStatCoefficient *= (1.0f + RemainRatio);
	}

	OutWaveMonsters.Sort([](const FWaveMonster& A, const FWaveMonster& B)
	{
		return A.Data.Gold < B.Data.Gold;
	});
	
	return OutWaveMonsters;
}

void AWaveManager::GetRandomMonsterByEnv(TSubclassOf<AMonster>& OutMonster, FMonsterData& OutData, FGameplayTag InEnvTag)
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
	Data->MonsterToData.GetKeys(Keys);

	if (Keys.Num() == 0) return;
	
	int32 RandomIndex = FMath::RandRange(0, Data->MonsterToData.Num() - 1);
	
	OutMonster = Keys[RandomIndex];
	OutData = Data->MonsterToData[OutMonster];
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

int32 AWaveManager::SpawnMonstersAndReturnGold(int32 SpawnAtOnce)
{
	if (WaveMonsters.IsEmpty() || !SpawnSpline) return 0;
	
	TArray<FVector> SpawnLocations
		= SpawnSpline->GetMonsterSpawnLocations(SpawnAtOnce);

	int32 UsingGold = 0;
	
	for (FVector SpawnLoc : SpawnLocations)
	{
		if (WaveMonsters.Num() == 0) return 0;

		// 첫 인덱스의 몬스터 픽
		FWaveMonster Monster = WaveMonsters[0];
		WaveMonsters.RemoveAt(0);
		if (!Monster.Class) continue;

		// 골드 +
		UsingGold += Monster.Data.Gold;

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
			Spawned->SetSkillSlots(Monster.Data.SkillSlots);
			Spawned->FinishSpawning(SpawnTransform);
			Spawned->GetAbilitySystemComponent()->SetNumericAttributeBase(UMAAttributeSet::GetCoinAttribute(), Monster.Data.Gold);
			Spawned->SetGoal(SpawnSpline);
			Spawned->OnMonsterDead.AddUObject(this, &AWaveManager::OnMonsterDead);
			AliveMonsterCount++;
		}

	}

	return UsingGold;
}

void AWaveManager::CreateBaseIntervalTimer()
{
	LastGold = TotalGold;

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

	int32 UsingGold =
		SpawnMonstersAndReturnGold(1);

	LastGold -= UsingGold;
	
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
