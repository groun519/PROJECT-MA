// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TransitionPlayerController.generated.h"

UCLASS()
class P_MA_API ATransitionPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void DelayedStartLoadingScreen();
	void TickLoadingStatus();

	FTimerHandle LoadingStatusTimerHandle;
};
