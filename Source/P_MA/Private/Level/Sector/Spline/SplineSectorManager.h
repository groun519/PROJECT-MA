// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SplineSector.h"
#include "GameFramework/Actor.h"
#include "Level/PlatformRoot.h"
#include "SplineSectorManager.generated.h"

UCLASS()
class P_MA_API ASplineSectorManager : public AActor
{
	GENERATED_BODY()

public:
	ASplineSectorManager();

	TObjectPtr<APlatformRoot> PlatformRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<ASplineSector>> Sectors;
	int32 PreSectorIndex = 0;

	bool IsClosePreSectorZeroVector();
	void GoBackToFirstSector();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
