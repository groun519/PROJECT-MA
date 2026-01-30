// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyGameState.generated.h"

class ALobbyAvatarSlot;
class AMAPlayerState;
class APlayerState;

USTRUCT()
struct FPlayerLobbySlot
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY()
	bool bReady = false;
};

UCLASS()
class P_MA_API ALobbyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void RegisterAvatarSlot(ALobbyAvatarSlot* Slot);
	void AssignSlotToPlayer(AMAPlayerState* PlayerState);
	void RemovePlayerFromSlot(AMAPlayerState* PlayerState);
	void SetPlayerReady(APlayerState* PlayerState, bool bReady);
	bool IsPlayerReady(const APlayerState* PlayerState) const;
	int32 GetSlotIndex(const APlayerState* PlayerState) const;
	int32 GetReadyCount() const;
	int32 GetPlayerCount() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY()
	TArray<TObjectPtr<ALobbyAvatarSlot>> AvatarSlots;

	UPROPERTY(ReplicatedUsing = OnRep_ReadyStates)
	TArray<FPlayerLobbySlot> LobbySlots;

	UFUNCTION()
	void OnRep_ReadyStates();
};
