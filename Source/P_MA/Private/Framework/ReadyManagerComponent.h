// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReadyManagerComponent.generated.h"

class AMAPlayerCharacter;
class AMAGameState;
class APlayerState;
class UReadyStateComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnReadyAllPlayersChanged, bool);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnReadyCountsChanged, int32, int32);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UReadyManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UReadyManagerComponent();

	void RefreshPlayerCache();
	void ResetAllPlayersReady();
	void GetReadyCounts(int32& OutReady, int32& OutTotal) const;
	void BroadcastReadyCounts();

	void SetPlayerLoopReady(APlayerState* PlayerState, bool bReady);
	bool IsPlayerLoopReady(const APlayerState* PlayerState) const;

	FORCEINLINE bool IsReady() const { return bAllPlayersReady; }

	FOnReadyAllPlayersChanged OnAllPlayersReadyChanged;
	FOnReadyCountsChanged OnReadyCountsChanged;

private:
	AMAGameState* GetMAGameState() const;
	AMAPlayerCharacter* FindPlayerCharacterByPlayerState(const APlayerState* PlayerState) const;
	UReadyStateComponent* FindReadyComponentByPlayerState(const APlayerState* PlayerState);
	const UReadyStateComponent* FindReadyComponentByPlayerState(const APlayerState* PlayerState) const;
	void SyncLoopReadyToGameState() const;
	void GetLoopReadyCounts(int32& OutReady, int32& OutTotal) const;
	void ResetAllLoopReady();

	TArray<TWeakObjectPtr<AMAPlayerCharacter>> CachedPlayers;
	bool bAllPlayersReady = false;
};
