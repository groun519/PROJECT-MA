// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "AI/Golem/Monster.h"
#include "Level/Sector/Battle/BattleSpaceSpline.h"
#include "Framework/MAGameMode.h"
#include "WaveManager.generated.h"

class UDataTable;

USTRUCT()
struct FWaveMonster
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<AMonster> Class;

	UPROPERTY()
	int32 Cost = 0;
};

UCLASS()
class P_MA_API AWaveManager : public AActor
{
	GENERATED_BODY()

public:
	AWaveManager();

protected:
	virtual void BeginPlay() override;
	void OnHandleGameStateChanged(EMAGameState NewState);

	/** Wave **/
public:
	UPROPERTY(EditAnywhere, Category = "Wave")
	ABattleSpaceSpline* SpawnSpline = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* MonsByEnvData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag CurEnvTag;

	TArray<FWaveMonster> WaveMonsters;

	void StartWave();
	// 웨이브 종료 및 관련변수 초기화
	void EndWave();
	bool bIsWaving = false;
	bool bWaveSpawnFinished = false;

	// 몬스터를 데이터에서 뽑아 배열에 저장
	TArray<FWaveMonster> GetNewWaveMonsters();
	// 몬스터 뽑기
	void GetRandomMonsterByEnv(TSubclassOf<AMonster>& OutMonster, int32& OutCost, FGameplayTag InEnvTag);

	// CostUnit 기반으로 BaseInterval을 나눔
	void CreateBaseIntervalTimer();
	void SpawnMonstersByInterval();
	// 개수만큼 몬스터 생성
	void SpawnMonsters(int32 SpawnAtOnce = 3);
	void OnMonsterDead();
	void TryEndWave();

private:
	UPROPERTY()
	AMAGameMode* CachedMAGameMode = nullptr;

	int32 Stage	= 1;
	int32 Wave = 1;
	int32 TotalWaveCost = 51;
	FORCEINLINE void SetTotalWaveCost(){ TotalWaveCost = 45 + Stage * 5 + Wave; }

	// 코스트 단위
	int32 CostUnit = 10;
	int32 LastCostUnit = 10;

	// 베이스 핸들
	FTimerHandle BaseIntervalTimerHandle;

	int32 AliveMonsterCount = 0;
};
