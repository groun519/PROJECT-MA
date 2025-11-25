// Fill out your copyright notice in the Description page of Project Settings.

#include "SplineSector.h"
#include "Components/SplineComponent.h"

ASplineSector::ASplineSector()
{
    /** Ground **/
    PCGExtentBox = CreateDefaultSubobject<UStaticMeshComponent>("GroundBox");
    SetRootComponent(PCGExtentBox);
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh
    (TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        PCGExtentBox->SetStaticMesh(CubeMesh.Object);
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

    /** PCG **/
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
    PCGComponent->InputType = EPCGComponentInput::Actor;
    PCGComponent->bParseActorComponents = true;
}

void ASplineSector::BeginPlay()
{
    Super::BeginPlay();
    //PCGComponent->Cleanup();
    SetRandomSeed();
}

void ASplineSector::UpdatePCGComponent()
{
    if (PCGComponent && PCGComponent->GetGraph())
    {
        PCGComponent->Seed = SectorSeed;
		UE_LOG(LogTemp, Warning, TEXT("PCG Sector Seed Updated!: %d"), SectorSeed);
        PCGComponent->Generate(true);
    }
}

void ASplineSector::UpdateSeed()
{
    if (Spline)
    {
        FVector StartPoint =
            FVector(
                -GetSectorBound().X,0.f,GetSectorBound().Z
                );

        FVector EndPoint =
            FVector(
                GetSectorBound().X,0.f,GetSectorBound().Z
                );

        Spline->ClearSplinePoints(false);
        Spline->AddSplinePoint(StartPoint, ESplineCoordinateSpace::Local);
        
        FRandomStream Stream(SectorSeed);
        
        for (int i = 1; i <= SplineNum-1; ++i)
        {
            float RandYBySplineSeed = Stream.FRandRange(-SplineOffset, SplineOffset);
            FVector Point = StartPoint + FVector(GetSectorBound().X * 2.f / SplineNum * i, RandYBySplineSeed, 0.f);
            Spline->AddSplinePoint(Point, ESplineCoordinateSpace::Local);
        }

        Spline->AddSplinePoint(EndPoint, ESplineCoordinateSpace::Local);

        Spline->ScaleVisualizationWidth = SplineWidth;
        int32 LastIndex = Spline->GetNumberOfSplinePoints() - 1;
        Spline->SetTangentAtSplinePoint(LastIndex, FVector(1,0,0), ESplineCoordinateSpace::Local);
        Spline->SetTangentAtSplinePoint(0, FVector(1,0,0), ESplineCoordinateSpace::Local);
    }
}

void ASplineSector::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (bRandomAtSpawn) SetRandomSeed();
}



void ASplineSector::SetSectorSeed(int32 InSeed)
{
    SectorSeed = InSeed;
    UE_LOG(LogTemp, Warning, TEXT("Copied New Seed at Last Sector!: %d"), InSeed);
    UpdateSeed();
    UpdatePCGComponent();
}

void ASplineSector::SetRandomSeed(int32 MaxValue)
{
    SectorSeed = FMath::RandRange(1, MaxValue);
    UpdateSeed();
    UpdatePCGComponent();
}

FVector ASplineSector::GetSectorBound()
{
    return PCGExtentBox->GetStaticMesh()->GetBounds().BoxExtent;
}


