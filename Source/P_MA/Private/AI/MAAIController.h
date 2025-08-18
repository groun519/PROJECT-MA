// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MAAIController.generated.h"

/**
 * 
 */
UCLASS()
class AMAAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMAAIController();

	virtual void OnPossess(APawn* NewPawn) override;
private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	class UAIPerceptionComponent* AIPerceptionComponent;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	class UAISenseConfig_Sight* SightConfig;
};
