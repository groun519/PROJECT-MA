// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformRoot.h"
#include "PlatformMatrixComponent.h"
#include "Components/SplineComponent.h"
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

void APlatformRoot::SetWaitMoveIn(bool bWaitMoveIn)
{
	// bool bWaitMoveIn =
	// 	CurState == EMAGameState::Wait || CurState == EMAGameState::EndBattle;
	PlatformMatrixComponent->SetMovedInPlatforms(bWaitMoveIn);
}

void APlatformRoot::SetHeight(bool bIsMoving)
{
	CurHeight = bIsMoving ? MovingHeight : WaitingHeight;
	UE_LOG(LogTemp, Warning, TEXT("Root: SetHeight -> %f"), CurHeight);
}

void APlatformRoot::SetCurSpline(USplineComponent* Spline)
{
	if (CurSpline != Spline)
	{
		CurSpline = Spline;
		Distance = 0.f;
		UE_LOG(LogTemp, Warning, TEXT("Root: SetCurSpline -> %s"), CurSpline ? *CurSpline->GetName() : TEXT("nullptr"));
	}
}

void APlatformRoot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/** TODO **//*
	 * 1. SetWaitMoveIn() when state change in manager
	 * 2. SetHeight() when state change in manager
	 * 3. SetCurSpline() when sector change in manager
	 */
	
	/** Set Height **/
	const float LocationInterpSpeed = 1.0f; 
	const float CurrentLocZ = GetActorLocation().Z;
	float SmoothedLocZ =
		FMath::FInterpTo(CurrentLocZ, CurHeight, DeltaTime, LocationInterpSpeed);
	FVector TargetZVec = GetActorLocation();
	TargetZVec.Z = SmoothedLocZ;
	SetActorLocation(TargetZVec);
	
	/** if Loop **/
	if (FMath::Abs(GetActorLocation().Z - CurHeight) > 10.f) return;

	if (!IsValid(CurSpline))
	{
		CurSpline = nullptr;
		return;
	}
	float Len = CurSpline->GetSplineLength();

	Distance += MoveSpeed * DeltaTime;

	if (Distance >= Len)
	{
		Distance -= Len;
		MoveEnd();
		if (!IsValid(CurSpline)) return;
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

void APlatformRoot::MoveEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("Root: ReachedEnd"));
	OnPlatformReachedEnd.Broadcast();
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
