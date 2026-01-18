// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "AI/Golem/Monster.h"
#include "Level/Sector/Battle/BattleSpaceSpline.h"
#include "MAGameMode.generated.h"


class UPCGGraph;

UENUM()
enum class EMAGameState : uint8
{
	// 멈춤
	Wait = 0,
	// 게임 시작 단계
	Start = 1,		

	/** Inf Loop **/

	// 전투 섹터 진입
	InBattle = 2,
	// 전투 중(웨이브)
	Battle = 3,
	// 전투 완료 후 모두 플랫폼에 올라타기 전까지 대기 단계
	EndBattle = 4,
	// 전투 섹터 빠져나가고 루프 진입까지
	OutBattle = 5,
	// 스플라인 섹터 무한반복하며 정비
	Loop = 6,		
};

USTRUCT()
struct FWaveMonster
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<AMonster> Class;

	UPROPERTY()
	int32 Cost = 0;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMAGameStateChanged, EMAGameState);
DECLARE_MULTICAST_DELEGATE(FOnAllPlayersReady);

/**
 * 
 */
UCLASS()
class AMAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
	virtual void BeginPlay() override;

	/** Delegate **/
	// 상태가 변했음을 매니저들에게 알려주는 델리게이트.
	FOnMAGameStateChanged OnMAGameStateChanged;
	// 플레이어들이 전부 레디 상태가 되었음을 알리는 델리게이트.
	FOnAllPlayersReady OnAllPlayersReady;
	
	/** Ready **/// test
	UPROPERTY(EditAnywhere)
	bool bAllPlayersReady = false;
	FORCEINLINE bool IsReady() const { return bAllPlayersReady; }
	
	/** State **/
	// 매니저들이 상태가 변했음을 게임모드에게 알릴 때 사용할 함수.
	void RequestStateChange(EMAGameState NewState);
	FORCEINLINE EMAGameState GetMAGameState() const { return MAGameState; } 

	/** Debug **/
	UFUNCTION(Exec)
	void SetMAState(int32 NewState);
	
private:
	EMAGameState MAGameState;

	AActor* FIndNextStartSpotForTeam(const FGenericTeamId& TeamID) const;

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartSpotTagMap;


	/** Map **/
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag CurEnvTag;
	
	
private:

	/** Wave **/
public:
	UPROPERTY(EditAnywhere, Category = "Wave")
	ABattleSpaceSpline* SpawnSpline;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDataTable* MonsByEnvData;

	TArray<FWaveMonster> WaveMonsters;

	void StartWave();
	// 웨이브 종료 및 관련변수 초기화
	void EndWave();
	bool bIsWaving = false;

	
	// 몬스터를 데이터에서 뽑아 배열에 저장
	TArray<FWaveMonster> GetNewWaveMonsters();
	// 몬스터 뽑기
	void GetRandomMonsterByEnv(TSubclassOf<AMonster>& OutMonster, int32& OutCost, FGameplayTag InEnvTag);

	// CostUnit 기반으로 BaseInterval을 나눔
	void CreateBaseIntervalTimer();
	void SpawnMonstersByInterval();
	// 개수만큼 몬스터 생성
	void SpawnMonsters(int32 SpawnAtOnce = 3);
	
	// CostUnit만큼 WaveMonsters에서 몬스터를 잘라내고 개수만큼 인터벌 생성
	//void CreateSchedulizedIntervalTimer(TArray<FVector> InSpawnTargetLoc); // 얘가 BaseIntervalTimer에 바인딩
	
private:
	int32 Stage	= 1;
	int32 Wave = 1;
	int32 TotalWaveCost = 51;
	FORCEINLINE void SetTotalWaveCost(){ TotalWaveCost = 45 + Stage * 5 + Wave; }

	// 코스트 단위
	int32 CostUnit = 10;
	int32 LastCostUnit = 10;

	// 베이스 핸들
	FTimerHandle BaseIntervalTimerHandle;
	// 스케줄 핸들
	// FTimerHandle SchedulizedIntervalTimerHandle;
};
