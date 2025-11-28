// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "MAGameMode.generated.h"


UENUM()
enum class EMAGameState : uint8
{
	Start,		// 게임 시작 단계

	/** Inf Loop **/
	InBattle,	// 전투 섹터 진입
	Battle,		// 전투 중(웨이브)
	EndBattle,	// 전투 완료 후 모두 플랫폼에 올라타기 전까지 대기 단계
	OutBattle,	// 전투 섹터 빠져나가고 루프 진입까지
	Loop,		// 스플라인 섹터 무한반복하며 정비
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

private:
	EMAGameState MAGameState;
};