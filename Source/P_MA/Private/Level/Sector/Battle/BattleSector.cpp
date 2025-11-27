// Fill out your copyright notice in the Description page of Project Settings.


#include "BattleSector.h"


ABattleSector::ABattleSector()
{
	PrimaryActorTick.bCanEverTick = true;

	InnerSpline = CreateDefaultSubobject<USplineComponent>(TEXT("InnerSpline"));
	InnerSpline->SetupAttachment(RootComponent);
    InnerSpline->ComponentTags.Add(FName("Battle"));
}

void ABattleSector::BeginPlay()
{
	Super::BeginPlay();
	UpdateInnerSpline();
}

void ABattleSector::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	if (bRandomAtSpawn) UpdateInnerSpline();
}

void ABattleSector::UpdateInnerSpline(int32 NumPoints)
{
	InnerSpline->ClearSplinePoints();
	for (int32 i = 0; i <= NumPoints; ++i)
	{
		float Angle = FMath::DegreesToRadians(i * (360.f / NumPoints));
		FVector PointOnCircle = FVector(FMath::Cos(Angle) * InnerSplineRadius, FMath::Sin(Angle) * InnerSplineRadius, 0.f);
		InnerSpline->AddSplinePoint(PointOnCircle, ESplineCoordinateSpace::Local);
	}
	InnerSpline->UpdateSpline();
}

