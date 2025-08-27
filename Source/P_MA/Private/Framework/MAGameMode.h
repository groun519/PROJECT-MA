// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GenericTeamAgentInterface.h"
#include "MAGameMode.generated.h"

/**
 * 
 */
UCLASS()
class AMAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual APlayerController* SpawnPlayerController(ENetRole InRemoteRole, const FString& Options) override;

private:
	FGenericTeamId GetTeamIDForPlayer(const APlayerController* PlayerController) const;

	AActor* FIndNextStartSpotForTeam(const FGenericTeamId& TeamID) const;

	UPROPERTY(EditDefaultsOnly, Category = "Team")
	TMap<FGenericTeamId, FName> TeamStartSpotTagMap;
};