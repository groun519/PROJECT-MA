// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "BattleSpaceSpline.generated.h"

UCLASS()
class P_MA_API ABattleSpaceSpline : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	ABattleSpaceSpline();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumPoints = 8;
	
	/** Spline **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USplineComponent> SpaceSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InnerSplineRadius = 1750.f;

	/** Spawn **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<APlayerStart*> PlayerStarts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StartDistanceOffset = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpawnSpawnPoint = true;

	TArray<FVector> GetMonsterSpawnLocations(int32 InNumPoints = 8);
	
private:
	void UpdateInnerSpline(int32 InNumPoints = 8);
};
