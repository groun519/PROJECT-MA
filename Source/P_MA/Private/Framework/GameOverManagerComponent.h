// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameOverManagerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UGameOverManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGameOverManagerComponent();

	void TryTriggerGameOver();
};
