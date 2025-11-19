// Fill out your copyright notice in the Description page of Project Settings.

#include "SplineSector.h"
#include "Components/SplineComponent.h"

ASplineSector::ASplineSector()
{
    /** Ground **/
    GroundBox = CreateDefaultSubobject<UStaticMeshComponent>("GroundBox");
    SetRootComponent(GroundBox);
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh
    (TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        GroundBox->SetStaticMesh(CubeMesh.Object);
    }

    /** Spline **/
    Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    Spline->SetupAttachment(RootComponent);

    /** Arrow **/
    Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
    Arrow->SetupAttachment(RootComponent);
    Arrow->SetRelativeScale3D(FVector(.02f, .02f, 1.f));
    Arrow->SetArrowSize(10.f);
    Arrow->SetArrowLength(100.f);
    Arrow->SetRelativeLocation(
        FVector(GetSectorBound().X,0.f,GetSectorBound().Z)
    );
    Arrow->SetArrowColor(FColor::Green);
}

void ASplineSector::BeginPlay()
{
    Super::BeginPlay();
}

void ASplineSector::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (bRandomAtSpawn) SetRandomSeed();
    
    if (Spline)
    {
        FVector StartPoint =
            FVector(
                -GetSectorBound().X,0.f,GetSectorBound().Z
                );

        Spline->ClearSplinePoints(false);
        Spline->AddSplinePoint(StartPoint, ESplineCoordinateSpace::Local);
        
        FRandomStream Stream(SplineSeed);
        
        for (int i = 1; i <= SplineNum - 1; ++i)
        {
            float RandYBySplineSeed = Stream.FRandRange(-SplineOffset, SplineOffset);
            FVector Point = StartPoint + FVector(GetSectorBound().X * 2.f / SplineNum * i, RandYBySplineSeed, 0.f);
            Spline->AddSplinePoint(Point, ESplineCoordinateSpace::Local);
        }
    }
}

void ASplineSector::SetRandomSeed(int MaxValue)
{
    SplineSeed = FMath::RandRange(1, MaxValue);
}

FVector ASplineSector::GetSectorBound()
{
    return GroundBox->GetStaticMesh()->GetBounds().BoxExtent;
}
