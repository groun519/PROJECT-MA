// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Abilities/GameplayAbility.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "MAPlayerState.generated.h"

UCLASS()
class P_MA_API AMAPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadoutColorChanged, const FMaterialParamDataPair&);

	void SetDefaultSkill(TSubclassOf<UGameplayAbility> NewSkill);
	TSubclassOf<UGameplayAbility> GetDefaultSkill() const { return DefaultSkill; }

	void SetLoadoutColor(const FMaterialParamDataPair& NewColor);
	const FMaterialParamDataPair& GetLoadoutColor() const { return LoadoutColor; }

	void SetLoadingComplete(bool bComplete);
	bool IsLoadingComplete() const { return bHasFinishedLoading; }

	void SetLobbySlotIndex(int32 Index);
	int32 GetLobbySlotIndex() const { return LobbySlotIndex; }

	FOnLoadoutColorChanged OnLoadoutColorChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_DefaultSkill)
	TSubclassOf<UGameplayAbility> DefaultSkill;

	UFUNCTION()
	void OnRep_DefaultSkill();

	UPROPERTY(ReplicatedUsing = OnRep_LoadoutColor)
	FMaterialParamDataPair LoadoutColor;

	UFUNCTION()
	void OnRep_LoadoutColor();

	UPROPERTY(ReplicatedUsing = OnRep_LoadingComplete)
	bool bHasFinishedLoading = false;

	UFUNCTION()
	void OnRep_LoadingComplete();

	UPROPERTY(ReplicatedUsing = OnRep_LobbySlotIndex)
	int32 LobbySlotIndex = INDEX_NONE;

	UFUNCTION()
	void OnRep_LobbySlotIndex();
};
