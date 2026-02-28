// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Framework/MAGameStateTypes.h"
#include "MAGameState.generated.h"

class APlayerState;
class AMAPlayerCharacter;

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
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnMASectorStateChanged, EMASectorState);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnStageCycleChanged, const FStageCycle&);

	AMAGameState();

	void SetMASectorState(EMASectorState NewState);
	EMASectorState GetMASectorState() const { return ReplicatedState; }
	FOnMASectorStateChanged OnMASectorStateChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** LoopReady **/
	void SyncLoopReadyEntries(const TArray<APlayerState*>& Players);
	void SetLoopReadyForPlayer(APlayerState* PlayerState, bool bReady);
	bool GetLoopReadyForPlayer(const APlayerState* PlayerState) const;
	void GetLoopReadyCounts(int32& OutReady, int32& OutTotal) const;
	void ResetLoopReadyEntries();
	const TArray<FLoopReadyEntry>& GetLoopReadyEntries() const { return LoopReadyEntries; }
	FOnLoopReadyEntriesChanged OnLoopReadyEntriesChanged;
	/**/
	
	/** Stage **/
	FOnStageCycleChanged OnStageCycleChanged;

	void SetStageCycle(const FStageCycle& NewStageCycle);
	const FStageCycle& GetStageCycle() const { return ReplicatedStageCycle; }
	void GetPlayerCharacters(TArray<AMAPlayerCharacter*>& OutPlayers, bool bAliveOnly = false) const;
	/**/

private:
	UPROPERTY(ReplicatedUsing=OnRep_MASectorState)
	EMASectorState ReplicatedState = EMASectorState::Wait;

	UFUNCTION()
	void OnRep_MASectorState();

	UPROPERTY(ReplicatedUsing=OnRep_LoopReadyEntries)
	TArray<FLoopReadyEntry> LoopReadyEntries;

	/** LoopReady **/
	UFUNCTION()
	void OnRep_LoopReadyEntries();
	/**/

	/** Stage **/
	UPROPERTY(ReplicatedUsing=OnRep_StageCycle)
	FStageCycle ReplicatedStageCycle;

	UFUNCTION()
	void OnRep_StageCycle();
	/**/
};
