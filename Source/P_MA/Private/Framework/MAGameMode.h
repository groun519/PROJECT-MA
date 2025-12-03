// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "MAGameMode.generated.h"


UENUM()
enum class EMAGameState : uint8
{
	// 멈춤
	Wait,
	// 게임 시작 단계
	Start,		

	/** Inf Loop **/

	// 전투 섹터 진입
	InBattle,
	// 전투 중(웨이브)
	Battle,
	// 전투 완료 후 모두 플랫폼에 올라타기 전까지 대기 단계
	EndBattle,
	// 전투 섹터 빠져나가고 루프 진입까지
	OutBattle,
	// 스플라인 섹터 무한반복하며 정비
	Loop,		
};


/**
 * 
 */
UCLASS()
class AMAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

private:
	AActor* FIndNextStartSpotForTeam(const FGenericTeamId& TeamID) const;

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartSpotTagMap;


	/** Map **/
public:
	FORCEINLINE EMAGameState GetMAGameState() const { return MAGameState; } 
	FORCEINLINE void SetMAGameState(EMAGameState InState) { MAGameState = InState; } 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag CurEnvTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMAGameState MAGameState;
private:

	/** Wave **/
public:
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	UDataAsset* WaveData = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	UDataTable* MonsterByEnvironmentData = nullptr;
		
private:
	int32 CurWave = 0;
};