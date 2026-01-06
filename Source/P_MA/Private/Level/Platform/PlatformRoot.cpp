// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformRoot.h"

#include "MovieSceneTracksComponentTypes.h"
#include "PlatformMatrixComponent.h"
#include "Level/Sector/Spline/SplineSectorManager.h"
#include "Level/Platform/Core.h"

APlatformRoot::APlatformRoot()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	/** Add Matrix **/
	PlatformMatrixComponent = CreateDefaultSubobject<UPlatformMatrixComponent>("Matrix");
	PlatformMatrixComponent->SetupAttachment(RootComponent);
}

void APlatformRoot::BeginPlay()
{
	Super::BeginPlay();
	SpawnCore();
	PlatformMatrixComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	PlatformMatrixComponent->InitMatrix();
}

void APlatformRoot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** Get SSManager **/
	ASplineSectorManager* Manager = ASplineSectorManager::FindSplineSectorManager(GetWorld());
	if (!Manager) return;

	/** Height **/
	Manager->IsMoving() ?
		CurHeight = MovingHeight :
		CurHeight = WaitingHeight;

	const float LocationInterpSpeed = 1.0f; 
	const float CurrentLocZ = GetActorLocation().Z;
	float SmoothedLocZ =
		FMath::FInterpTo(CurrentLocZ, CurHeight, DeltaTime, LocationInterpSpeed);
	FVector TargetZVec = GetActorLocation();
	TargetZVec.Z = SmoothedLocZ;
	SetActorLocation(TargetZVec);
	
	/** if Loop **/
	if (Manager->CurSectors.Num() == 0)
	{
		AMAGameMode* MAGM = Manager->GetMAGameMode();
		if (MAGM)
		{
			Manager->SetSplinesWithMAGameState(MAGM->GetMAGameState());
		}
		return;
	}

	if (FMath::Abs(GetActorLocation().Z - CurHeight) > 10.f) return;

	USplineComponent* CurSpline = Manager->CurSectors[CurSector]->RoadSpline;
	float Len = CurSpline->GetSplineLength();

	Distance += MoveSpeed * DeltaTime;

	if (Distance >= Len)
	{
		Distance -= Len;
		CurSector++;

		if (CurSector >= Manager->CurSectors.Num())
		{
			CurSector = 0;
			Distance  = 0.f;

			AMAGameMode* MAGM = Manager->GetMAGameMode();
			Manager->SetSplinesWithMAGameState(
				MAGM->GetMAGameState());
			if (Manager->CurSectors.Num() == 0) return;
		}

		int32 NewSectorIndex = Manager->GetNextSectorIndex(CurSector);
		Manager->CurSectors[NewSectorIndex]->SetRandomSeed();
		
		CurSpline = Manager->CurSectors[CurSector]->RoadSpline;
	}

	FVector TargetLoc =
		CurSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	FRotator TargetRot =
		CurSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	TargetRot.Pitch = 0.f;
	TargetRot.Roll  = 0.f;

	const float RotationInterpSpeed = 1.0f; 
	const FRotator CurrentRot = GetActorRotation();
	const FRotator SmoothedRot =
		FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotationInterpSpeed);

	TargetLoc.Z = GetActorLocation().Z;

	SetActorLocation(TargetLoc);
	SetActorRotation(SmoothedRot);
}

void APlatformRoot::SpawnCore()
{
	if (!GetWorld() || !CoreClass) return;
	
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Core = GetWorld()->SpawnActor<ACore>(CoreClass, GetActorTransform(), Params);
	if (Core)
	{
		Core->AttachToComponent(
			Root,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale
		);
	}
	Core->SetActorRelativeLocation(FVector(0, 0, 100.f));
}