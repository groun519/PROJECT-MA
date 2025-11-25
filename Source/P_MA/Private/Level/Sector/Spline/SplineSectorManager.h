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

	UPROPERTY()
	TObjectPtr<APlatformRoot> PlatformRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<ASplineSector>> Sectors;

	bool IsClosePreSectorZeroVector();
	void GoBackToFirstSector();

	int32 GetNextSectorIndex(int32 CurSectorIndex);
	static ASplineSectorManager* FindSplineSectorManager(UWorld* World);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

};
