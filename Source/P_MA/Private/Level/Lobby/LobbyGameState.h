#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LobbyAvatarState.h"
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
	DECLARE_MULTICAST_DELEGATE(FLobbySlotsRegistered);
	FLobbySlotsRegistered OnSlotsRegistered;

	void RegisterAvatarSlot(ALobbyAvatarSlot* Slot);
	void AssignSlotToPlayer(AMAPlayerState* PlayerState);
	void RemovePlayerFromSlot(AMAPlayerState* PlayerState);
	void SetPlayerReady(APlayerState* PlayerState, bool bReady);
	void SetPlayerLobbyState(APlayerState* PlayerState, ELobbyAvatarState NewState);
	void RefreshPlayerLobbyState(APlayerState* PlayerState);
	bool IsPlayerReady(const APlayerState* PlayerState) const;
	int32 GetSlotIndex(const APlayerState* PlayerState) const;
	ALobbyAvatarSlot* GetAvatarSlot(int32 Index) const;
	const TArray<TObjectPtr<ALobbyAvatarSlot>>& GetAvatarSlots() const { return AvatarSlots; }
	int32 GetReadyCount() const;
	int32 GetPlayerCount() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	int32 DefaultLobbySlotCount = 4;

	UPROPERTY()
	TArray<TObjectPtr<ALobbyAvatarSlot>> AvatarSlots;

	UPROPERTY(ReplicatedUsing = OnRep_LobbySlots)
	TArray<FPlayerLobbySlot> LobbySlots;

	UPROPERTY(ReplicatedUsing = OnRep_LobbyStates)
	TArray<ELobbyAvatarState> LobbyStates;

	UPROPERTY()
	TArray<TWeakObjectPtr<APlayerState>> LobbySlotPlayersCache;

	UFUNCTION()
	void OnRep_LobbySlots();

	UFUNCTION()
	void OnRep_LobbyStates();

	int32 GetDesiredSlotCount() const;
	void EnsureSlotStorageSize(int32 DesiredCount);
	void ApplyLobbySlotsToAvatars();
};
