#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Abilities/GameplayAbility.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "MAPlayerState.generated.h"

UCLASS()
class P_MA_API AMAPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OverrideWith(APlayerState* PlayerState) override;

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoadoutChanged, const FLoadoutSelection&);

	void SetDefaultSkill(TSubclassOf<UGameplayAbility> NewSkill);
	TSubclassOf<UGameplayAbility> GetDefaultSkill() const { return DefaultSkill; }

	const FMaterialParamDataPair& GetLoadoutColor() const { return LoadoutSelection.Color; }

	void SetLoadoutSelection(const FLoadoutSelection& NewLoadout);
	const FLoadoutSelection& GetLoadoutSelection() const;

	FName GetLoadoutWeaponId() const { return LoadoutSelection.WeaponId; }

	FName GetLoadoutEyeShapeId() const { return LoadoutSelection.EyeShapeId; }

	FName GetLoadoutMountId() const { return LoadoutSelection.MountId; }

	void SetLoadingComplete(bool bComplete);
	bool IsLoadingComplete() const { return bHasFinishedLoading; }

	void SetLobbySlotIndex(int32 Index);
	int32 GetLobbySlotIndex() const { return LobbySlotIndex; }

	FOnLoadoutChanged OnLoadoutChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_DefaultSkill)
	TSubclassOf<UGameplayAbility> DefaultSkill;

	UFUNCTION()
	void OnRep_DefaultSkill();

	UPROPERTY(ReplicatedUsing = OnRep_LoadoutSelection)
	FLoadoutSelection LoadoutSelection;

	UFUNCTION()
	void OnRep_LoadoutSelection();

	UPROPERTY(ReplicatedUsing = OnRep_LoadingComplete)
	bool bHasFinishedLoading = false;

	UFUNCTION()
	void OnRep_LoadingComplete();

	UPROPERTY(ReplicatedUsing = OnRep_LobbySlotIndex)
	int32 LobbySlotIndex = INDEX_NONE;

	UFUNCTION()
	void OnRep_LobbySlotIndex();
};
