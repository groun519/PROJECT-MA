// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleSpaceSpline.h"
#include "Components/SplineComponent.h"


ABattleSpaceSpline::ABattleSpaceSpline()
{
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(FName("BattleSpace"));

	SpaceSpline = CreateDefaultSubobject<USplineComponent>(TEXT("SpaceSpline"));
	SpaceSpline->SetupAttachment(RootComponent);
	SpaceSpline->SetClosedLoop(true);
}

void ABattleSpaceSpline::BeginPlay()
{
	Super::BeginPlay();
	UpdateInnerSpline(NumPoints);
}

void ABattleSpaceSpline::UpdateInnerSpline(int32 InNumPoints)
{
	SpaceSpline->ClearSplinePoints();
	for (int32 i = 0; i < InNumPoints; ++i)
	{
		/** Add SplinePoint **/
		float Angle = FMath::DegreesToRadians(i * (360.f / InNumPoints));
		FVector SplinePointOnCircle = FVector(FMath::Cos(Angle) * InnerSplineRadius, FMath::Sin(Angle) * InnerSplineRadius, 0.f);
		SpaceSpline->AddSplinePoint(SplinePointOnCircle, ESplineCoordinateSpace::Local);
	}
	SpaceSpline->UpdateSpline();
}

TArray<FVector> ABattleSpaceSpline::GetMonsterSpawnLocations(int32 InNumPoints)
{
	TArray<FVector> SpawnLocations;
	float MonsterSpawnRadius = InnerSplineRadius + 250.f;
	for (int32 i = 0; i < InNumPoints; ++i)
	{
		/** Get Points **/
		float Angle = FMath::DegreesToRadians(i * (360.f / InNumPoints));
		FVector SplinePointOnCircle = FVector(FMath::Cos(Angle) * MonsterSpawnRadius, FMath::Sin(Angle) * MonsterSpawnRadius, 0.f);
		SpawnLocations.Add(SplinePointOnCircle);
	}
	return SpawnLocations;
}

