// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Framework/MAGameStateTypes.h"
#include "MAGameState.generated.h"

class APlayerState;

USTRUCT()
struct FLoopReadyEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY()
	bool bReady = false;
};

UCLASS()
class P_MA_API AMAGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE(FOnLoopReadyEntriesChanged);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMAGameStateChanged, EMAGameState);

	AMAGameState();

	void SetMAGameState(EMAGameState NewState);
	EMAGameState GetMAGameState() const { return ReplicatedState; }

	void SyncLoopReadyEntries(const TArray<APlayerState*>& Players);
	void SetLoopReadyForPlayer(APlayerState* PlayerState, bool bReady);
	bool GetLoopReadyForPlayer(const APlayerState* PlayerState) const;
	void GetLoopReadyCounts(int32& OutReady, int32& OutTotal) const;
	const TArray<FLoopReadyEntry>& GetLoopReadyEntries() const { return LoopReadyEntries; }
	FOnLoopReadyEntriesChanged OnLoopReadyEntriesChanged;
	FOnMAGameStateChanged OnMAGameStateChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing=OnRep_MAGameState)
	EMAGameState ReplicatedState = EMAGameState::Wait;

	UFUNCTION()
	void OnRep_MAGameState();

	UPROPERTY(ReplicatedUsing=OnRep_LoopReadyEntries)
	TArray<FLoopReadyEntry> LoopReadyEntries;

	UFUNCTION()
	void OnRep_LoopReadyEntries();

};
