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
	// Seamless travel 시 PlayerState가 교체/재구성될 수 있어
	// 커스텀 로드아웃 데이터가 유실되지 않도록 명시적으로 복사한다.
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OverrideWith(APlayerState* PlayerState) override;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadoutColorChanged, const FMaterialParamDataPair&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadoutWeaponChanged, FName);

	void SetDefaultSkill(TSubclassOf<UGameplayAbility> NewSkill);
	TSubclassOf<UGameplayAbility> GetDefaultSkill() const { return DefaultSkill; }

	void SetLoadoutColor(const FMaterialParamDataPair& NewColor);
	const FMaterialParamDataPair& GetLoadoutColor() const { return LoadoutColor; }

	void SetLoadoutWeaponId(FName NewWeaponId);
	FName GetLoadoutWeaponId() const { return LoadoutWeaponId; }

	void SetLoadingComplete(bool bComplete);
	bool IsLoadingComplete() const { return bHasFinishedLoading; }

	void SetLobbySlotIndex(int32 Index);
	int32 GetLobbySlotIndex() const { return LobbySlotIndex; }


	FOnLoadoutColorChanged OnLoadoutColorChanged;
	FOnLoadoutWeaponChanged OnLoadoutWeaponChanged;

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

	UPROPERTY(ReplicatedUsing = OnRep_LoadoutWeaponId)
	FName LoadoutWeaponId = TEXT("1");

	UFUNCTION()
	void OnRep_LoadoutWeaponId();

	UPROPERTY(ReplicatedUsing = OnRep_LoadingComplete)
	bool bHasFinishedLoading = false;

	UFUNCTION()
	void OnRep_LoadingComplete();

	UPROPERTY(ReplicatedUsing = OnRep_LobbySlotIndex)
	int32 LobbySlotIndex = INDEX_NONE;

	UFUNCTION()
	void OnRep_LobbySlotIndex();

};
