// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "Framework/MAGameStateTypes.h"
#include "MAGameMode.generated.h"


class UPCGGraph;
class AMAGameState;
class APlayerState;
class UReadyManagerComponent;
class UGameOverManagerComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMASectorStateChanged, EMASectorState);
DECLARE_MULTICAST_DELEGATE(FOnAllPlayersReady);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnReadyCountChanged, int32, int32);

/**
 * 
 */
UCLASS()
class AMAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMAGameMode();
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** Delegate **/
	// 상태가 변했음을 매니저들에게 알려주는 델리게이트.
	FOnMASectorStateChanged OnMASectorStateChanged;
	// 플레이어들이 전부 레디 상태가 되었음을 알리는 델리게이트.
	FOnAllPlayersReady OnAllPlayersReady;
	// 레디 인원 변화를 알리는 델리게이트.
	FOnReadyCountChanged OnReadyCountChanged;
	
	/** Ready **/// test
	UPROPERTY(EditAnywhere)
	bool bAllPlayersReady = false;
	FORCEINLINE bool IsReady() const { return bAllPlayersReady; }

	/** Loop Ready (TODO: move to dedicated manager later) **/
	void SetPlayerLoopReady(APlayerState* PlayerState, bool bReady);
	bool IsPlayerLoopReady(const APlayerState* PlayerState) const;
	
	/** State **/
	// 매니저들이 상태가 변했음을 게임모드에게 알릴 때 사용할 함수.
	void RequestStateChange(EMASectorState NewState);
	EMASectorState GetNextState(EMASectorState CurState) const;
	void RequestNextState(EMASectorState CurState);
	FORCEINLINE EMASectorState GetMASectorState() const { return CurrentMASectorState; } 
	void RefreshPlayerCache();
	void ResetAllPlayersReady();
	void GetReadyCounts(int32& OutReady, int32& OutTotal) const;
	void BroadcastReadyCounts();

	/** Debug **/
	UFUNCTION(Exec)
	void SetMAState(int32 NewState);
	
private:
	EMASectorState CurrentMASectorState;

	UPROPERTY(VisibleAnywhere, Category = "Ready")
	TObjectPtr<UReadyManagerComponent> ReadyManagerComponent = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Game Over")
	TObjectPtr<UGameOverManagerComponent> GameOverManagerComponent = nullptr;

	AActor* FIndNextStartSpotForTeam(const FGenericTeamId& TeamID) const;
	void HandleReadyCountsChanged(int32 ReadyCount, int32 TotalCount);
	void HandleAllPlayersReadyChanged(bool bIsAllReady);

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartSpotTagMap;
};
