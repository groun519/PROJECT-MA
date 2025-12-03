// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleSpaceSpline.h"
#include "Components/SplineComponent.h"
#include "GameFramework/PlayerStart.h"


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
