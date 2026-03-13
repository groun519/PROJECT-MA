// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyAvatarState.h"
#include "Player/Loadout/LoadoutTypes.h"
#include "LobbyAvatarSlot.generated.h"

class USkeletalMeshComponent;
class USpotLightComponent;
class UWidgetComponent;
class AMAPlayerState;
class UMaterialInstanceDynamic;

UCLASS()
class P_MA_API ALobbyAvatarSlot : public AActor
{
	GENERATED_BODY()

public:
	ALobbyAvatarSlot();
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lobby")
	int32 SlotIndex = 0;

	void SetOccupant(AMAPlayerState* NewPlayerState);
	void SetLocalHidden(bool bHide);
	void ApplyLoadoutColor(const FMaterialParamDataPair& ColorData);
	void ApplyLoadoutEyeShape(FName EyeShapeId);
	void ApplyLoadoutMountId(FName MountId);
	void SetMountPreviewVisible(bool bVisible);
	void SetWeaponPreviewVisible(bool bVisible);
	void SetLobbyState(ELobbyAvatarState State);
	void SetWeaponOnlyOwnerSee(bool bEnable);
	void ApplyLoadoutWeaponId(FName WeaponId);
	void ApplyLoadoutWeaponMesh(USkeletalMesh* Mesh, const FTransform& Offset);

private:
	void HandleLoadoutChanged(const FLoadoutSelection& Loadout);

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* AvatarMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* WeaponMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* MountMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	USpotLightComponent* AvatarSpotLight = nullptr;

	UPROPERTY(EditAnywhere, Category = "Lobby")
	FName WeaponSocketName = TEXT("WeaponHandSocket");

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* NameWidget = nullptr;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* ReadyWidget = nullptr;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* InviteWidget = nullptr;

	UPROPERTY()
	TObjectPtr<AMAPlayerState> Occupant;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> AvatarDynMat;
	FEyeShapeParamData CurrentEyeShapeData;

	FDelegateHandle LoadoutChangedHandle;
	bool bMountPreviewVisible = false;
	bool bWeaponPreviewVisible = true;
};
