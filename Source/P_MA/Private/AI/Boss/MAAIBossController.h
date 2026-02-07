// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/MAAIController.h"
#include "MAAIBossController.generated.h"

/**
 * 
 */
UCLASS()
class AMAAIBossController : public AMAAIController
{
	GENERATED_BODY()

public:
	AMAAIBossController();

protected:
	virtual void OnPossess(APawn* NewPawn) override;
};
