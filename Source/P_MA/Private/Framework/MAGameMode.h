// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "MAGameMode.generated.h"


class UPCGGraph;
class AMAPlayerCharacter;

UENUM()
enum class EMAGameState : uint8
{
	// 멈춤
	Wait = 0,
	// 게임 시작 단계. 루프 탈출 시에도 사용.
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
	virtual void PostLogin(APlayerController* NewPlayer) override;

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
	void RefreshPlayerCache();
	void ResetAllPlayersReady();

	/** Debug **/
	UFUNCTION(Exec)
	void SetMAState(int32 NewState);
	
private:
	EMAGameState MAGameState;
	TArray<TWeakObjectPtr<AMAPlayerCharacter>> CachedPlayers;

	AActor* FIndNextStartSpotForTeam(const FGenericTeamId& TeamID) const;

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartSpotTagMap;
};
