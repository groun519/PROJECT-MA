// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformRoot.h"

#include "PlatformComponent.h"
#include "PlatformMatrixComponent.h"
#include "Components/ArrowComponent.h"
#include "Sector/Spline/SplineSectorManager.h"

APlatformRoot::APlatformRoot()
{
	PrimaryActorTick.bCanEverTick = true;

	/** Add Matrix **/
	PlatformMatrixComponent = CreateDefaultSubobject<UPlatformMatrixComponent>("Matrix");
	PlatformMatrixComponent->SetupAttachment(RootComponent);
	
	/** Add Arrow **/
	if (UArrowComponent* Arrow = GetArrowComponent())
	{
		Arrow->ArrowSize = 3.0f;
		Arrow->ArrowColor = FColor::Red;
		Arrow->SetRelativeLocation(FVector(1000.0f, 0.0f, 50.f));
	}
}

void APlatformRoot::BeginPlay()
{
	Super::BeginPlay();
}

void APlatformRoot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ASplineSectorManager* Manager = ASplineSectorManager::FindSplineSectorManager(GetWorld());
	if (!Manager) return;

	if (Manager->Sectors.Num() == 0) return;

	USplineComponent* CurSpline = Manager->Sectors[CurSector]->RoadSpline;
	float Len = CurSpline->GetSplineLength();

	Distance += MoveSpeed * DeltaTime;

	if (Distance >= Len)
	{
		Distance -= Len;
		CurSector++;

		if (CurSector >= Manager->Sectors.Num())
		{
			CurSector = 0;
			Distance  = 0.f;
		}

		int32 NewSectorIndex = Manager->GetNextSectorIndex(CurSector);
		Manager->Sectors[NewSectorIndex]->SetRandomSeed();
		
		CurSpline = Manager->Sectors[CurSector]->RoadSpline;
	}

	FVector Loc = CurSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
	FRotator Rot = CurSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	SetActorLocation(Loc);
	SetActorRotation(Rot);
}

void APlatformRoot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

