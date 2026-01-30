// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyAvatarSlot.generated.h"

class USkeletalMeshComponent;
class USpotLightComponent;
class UWidgetComponent;
class AMAPlayerState;

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

private:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root = nullptr;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* AvatarMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* WeaponMesh = nullptr;

	UPROPERTY(VisibleAnywhere)
	USpotLightComponent* AvatarSpotLight = nullptr;

	UPROPERTY(EditAnywhere, Category = "Lobby")
	FName WeaponSocketName = TEXT("WeaponHandSocket");

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* NameWidget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Lobby")
	TSubclassOf<class ULobbyAvatarNameWidget> NameWidgetClass;

	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* InviteWidget = nullptr;

	UPROPERTY(EditAnywhere, Category = "Lobby")
	TSubclassOf<class ULobbyInviteWidget> InviteWidgetClass;

	UPROPERTY()
	TObjectPtr<AMAPlayerState> Occupant;
};
