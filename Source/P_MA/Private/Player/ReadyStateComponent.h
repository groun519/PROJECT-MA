// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReadyStateComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UReadyStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UReadyStateComponent();

	void SetReady(bool bNewReady);
	FORCEINLINE bool IsReady() const { return bIsReady; }

private:
	bool bIsReady = false;
};
