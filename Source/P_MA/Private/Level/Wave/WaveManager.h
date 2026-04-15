#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "AI/Golem/Monster.h"
#include "Level/Sector/Battle/BattleSpaceSpline.h"
#include "Framework/MAGameStateTypes.h"
#include "WaveManager.generated.h"

class UDataTable;
class AMAGameMode;

USTRUCT()
struct FWaveMonster
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<AMonster> Class;

	UPROPERTY()
	int32 Gold = 0;
};

USTRUCT(BlueprintType)
struct FWaveSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 BaseGold = 1000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 AddingGoldPerWave = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 MaxMonsterNum = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float MonsterStatCoefficient = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float AddingMonsterStatCoefficientPerWave = 0.01;
};

UCLASS()
class P_MA_API AWaveManager : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	void OnHandleSectorStateChanged(EMASectorState NewState);

public:
	AWaveManager();
	
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
	void GetRandomMonsterByEnv(TSubclassOf<AMonster>& OutMonster, int32& OutGold, FGameplayTag InEnvTag);

	void CreateBaseIntervalTimer();
	// 개수만큼 몬스터 생성 후 골드량 리턴
	int32 SpawnMonstersAndReturnGold(int32 SpawnAtOnce = 3);
	void OnMonsterDead();
	void TryEndWave();

private:
	// 몬스터 스폰
	FTimerHandle BaseIntervalTimerHandle;
	float SpawnInterval = 1.f;
	void SpawnMonstersByInterval();
	
	UPROPERTY()
	AMAGameMode* CachedMAGameMode = nullptr;

	void OnEnvironmentChanged(const FGameplayTag& NewEnvTag);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave", meta = (AllowPrivateAccess = "true"))
	FWaveSetting WaveSetting;
	
	int32 TotalGold = 0;
	int32 Wave = 1;
	float MonsterStatCoefficient = 1.0;
	int32 LastGold = 0;
	
	FORCEINLINE void SetTotalGoldByWave()
	{
		TotalGold =
			WaveSetting.BaseGold + WaveSetting.AddingGoldPerWave * Wave;
	}
	FORCEINLINE void SetStatCoefficientByWave()
	{
		MonsterStatCoefficient =
			WaveSetting.MonsterStatCoefficient + WaveSetting.AddingMonsterStatCoefficientPerWave * Wave;
	}
	
	int32 AliveMonsterCount = 0;

	/** Init **/
	bool InitCachedMAGameMode();
	bool InitSpawnSpline();
	bool BindEnvironmentManager();
};
