// Fill out your copyright notice in the Description page of Project Settings.

#include "SplineSector.h"
#include "Components/SplineComponent.h"
#include "Net/UnrealNetwork.h"

ASplineSector::ASplineSector()
{
    bReplicates = true;

    /** Ground **/
    PCGExtentBox = CreateDefaultSubobject<UStaticMeshComponent>("GroundBox");
    SetRootComponent(PCGExtentBox);
    
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh
    (TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        PCGExtentBox->SetStaticMesh(CubeMesh.Object);
    }
    
    PCGExtentBox->SetWorldScale3D(FVector(63.f));
    PCGExtentBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    PCGExtentBox->SetVisibility(false);
    PCGExtentBox->SetGenerateOverlapEvents(false);
    PCGExtentBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PCGExtentBox->SetCollisionObjectType(ECC_WorldStatic);
    PCGExtentBox->CanCharacterStepUpOn = ECB_No;
    
    /** Spline **/
    RoadSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    RoadSpline->SetupAttachment(RootComponent);
    RoadSpline->ComponentTags.Add(FName("Road"));

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
    PCGComponent->SetIsPartitioned(false);
}

void ASplineSector::BeginPlay()
{
    Super::BeginPlay();
    //PCGComponent->Cleanup();
    if (HasAuthority())
    {
        SetRandomSeed();
    }
}

void ASplineSector::UpdatePCGComponent()
{
    if (PCGComponent && PCGComponent->GetGraph())
    {
        PCGComponent->Seed = SectorSeed;
        PCGComponent->Generate(true);
    }
}

void ASplineSector::UpdateSeed()
{
    if (RoadSpline)
    {
        FVector StartPoint =
            FVector(
                -GetSectorBound().X,0.f,GetSectorBound().Z
                );

        FVector EndPoint =
            FVector(
                GetSectorBound().X,0.f,GetSectorBound().Z
                );

        RoadSpline->ClearSplinePoints(false);
        RoadSpline->AddSplinePoint(StartPoint, ESplineCoordinateSpace::Local);
        
        FRandomStream Stream(SectorSeed);
        
        for (int i = 1; i <= SplineNum-1; ++i)
        {
            float RandYBySplineSeed = Stream.FRandRange(-SplineOffset, SplineOffset);
            FVector Point = StartPoint + FVector(GetSectorBound().X * 2.f / SplineNum * i, RandYBySplineSeed, 0.f);
            RoadSpline->AddSplinePoint(Point, ESplineCoordinateSpace::Local);
        }

        RoadSpline->AddSplinePoint(EndPoint, ESplineCoordinateSpace::Local);

        int32 LastIndex = RoadSpline->GetNumberOfSplinePoints() - 1;
        RoadSpline->SetTangentAtSplinePoint(LastIndex, FVector(1,0,0), ESplineCoordinateSpace::Local);
        RoadSpline->SetTangentAtSplinePoint(0, FVector(1,0,0), ESplineCoordinateSpace::Local);
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
    if (!HasAuthority()) return;

    SectorSeed = FMath::RandRange(1, MaxValue);
    UpdateSeed();
    UpdatePCGComponent();
}

void ASplineSector::OnRep_SectorSeed()
{
    UpdateSeed();
    UpdatePCGComponent();
}

void ASplineSector::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ASplineSector, SectorSeed);
}

FVector ASplineSector::GetSectorBound()
{
    if (!PCGExtentBox || !PCGExtentBox->GetStaticMesh()) return  FVector::ZeroVector;
    return PCGExtentBox->GetStaticMesh()->GetBounds().BoxExtent;
}
