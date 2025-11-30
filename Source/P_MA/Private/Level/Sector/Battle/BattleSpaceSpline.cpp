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
	UpdateInnerSpline();
}

void ABattleSpaceSpline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (bRandomAtSpawn) UpdateInnerSpline();
}

void ABattleSpaceSpline::UpdateInnerSpline(int32 NumPoints)
{
	SpaceSpline->ClearSplinePoints();
	for (int32 i = 0; i < NumPoints; ++i)
	{
		float Angle = FMath::DegreesToRadians(i * (360.f / NumPoints));
		FVector PointOnCircle = FVector(FMath::Cos(Angle) * InnerSplineRadius, FMath::Sin(Angle) * InnerSplineRadius, 0.f);
		SpaceSpline->AddSplinePoint(PointOnCircle, ESplineCoordinateSpace::Local);
	}
	SpaceSpline->UpdateSpline();
}
