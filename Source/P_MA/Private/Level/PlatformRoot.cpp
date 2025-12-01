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
	
	// /** Add Arrow **/
	// if (UArrowComponent* Arrow = GetArrowComponent())
	// {
	// 	Arrow->ArrowSize = 3.0f;
	// 	Arrow->ArrowColor = FColor::Red;
	// 	Arrow->SetRelativeLocation(FVector(1000.0f, 0.0f, 50.f));
	// }
}

void APlatformRoot::BeginPlay()
{
	Super::BeginPlay();
}

void APlatformRoot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** Get SSManager **/
	ASplineSectorManager* Manager = ASplineSectorManager::FindSplineSectorManager(GetWorld());
	if (!Manager) return;

	/** if Loop **/
	if (Manager->Sectors.Num() == 0)
	{
		AMAGameMode* MAGM = Manager->GetMAGameMode();
		Manager->SetSplinesWithMAGameState(
			MAGM->GetMAGameState());
		return;
	}

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

			AMAGameMode* MAGM = Manager->GetMAGameMode();
			Manager->SetSplinesWithMAGameState(
				MAGM->GetMAGameState());
			if (Manager->Sectors.Num() == 0) return;
		}

		int32 NewSectorIndex = Manager->GetNextSectorIndex(CurSector);
		Manager->Sectors[NewSectorIndex]->SetRandomSeed();
		
		CurSpline = Manager->Sectors[CurSector]->RoadSpline;
	}

	const FVector TargetLoc =
		CurSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	FRotator TargetRot =
		CurSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	TargetRot.Pitch = 0.f;
	TargetRot.Roll  = 0.f;

	const float RotationInterpSpeed = 1.0f; 

	const FRotator CurrentRot = GetActorRotation();

	const FRotator SmoothedRot =
		FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationInterpSpeed);

	SetActorLocation(TargetLoc);
	SetActorRotation(SmoothedRot);
}
