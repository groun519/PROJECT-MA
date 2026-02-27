// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Framework/MAGameStateTypes.h"
#include "GameOverManagerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UGameOverManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameOverManagerComponent();

	void TryTriggerGameOver();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void HandleSectorStateChanged(EMASectorState NewState);
	void ReviveAllDeadPlayers();

	class AMAGameMode* GetOwnerGameMode() const;
	class AMAGameState* GetMAGameState() const;
};
