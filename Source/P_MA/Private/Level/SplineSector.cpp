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
}

void ASplineSector::BeginPlay()
{
    Super::BeginPlay();

    if (Spline)
    {
        FVector Bounds = GroundBox->GetStaticMesh()->GetBounds().BoxExtent;
        FVector StartPoint =
            FVector(
                Bounds.X / 2.f,0.f,Bounds.Z / 2.f
                );
        FVector EndPoint =
            FVector(
                -Bounds.X / 2.f,0.f,Bounds.Z / 2.f
                );
        
        Spline->AddSplinePoint(StartPoint, ESplineCoordinateSpace::Local);
        Spline->AddSplinePoint(EndPoint, ESplineCoordinateSpace::Local);

        // for (int i = 1; i < 10; ++i)  
        // {
        //     FVector RandomPoint = FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), 0);
        //     Spline->AddSplinePoint(RandomPoint, ESplineCoordinateSpace::Local);
        // }

        // float TotalLength = Spline->GetSplineLength();
        // if (TotalLength > MaxLength)
        // {
        //     FVector AdjustedEnd = StartPoint + (EndPoint - StartPoint).GetSafeNormal() * MaxLength;
        //     int LastPointIndex = Spline->GetNumberOfSplinePoints() - 1;
        //     Spline->SetLocationAtSplinePoint(LastPointIndex, AdjustedEnd, ESplineCoordinateSpace::Local);
        // }
    }
}
