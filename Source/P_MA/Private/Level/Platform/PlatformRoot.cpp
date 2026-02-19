// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformRoot.h"
#include "PlatformMatrixComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/SplineComponent.h"
#include "Level/Platform/Core.h"
#include "Net/UnrealNetwork.h"

APlatformRoot::APlatformRoot()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	{
		FRepMovement RepMove = GetReplicatedMovement();
		RepMove.RotationQuantizationLevel = ERotatorQuantization::ShortComponents;
		SetReplicatedMovement(RepMove);
	}

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	
	/** Add Matrix **/
	PlatformMatrixComponent = CreateDefaultSubobject<UPlatformMatrixComponent>("Matrix");
	PlatformMatrixComponent->SetupAttachment(RootComponent);

	/** Ready Text **/
	ReadyText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("ReadyText"));
	ReadyText->SetupAttachment(RootComponent);
	ReadyText->SetHorizontalAlignment(EHTA_Center);
	ReadyText->SetVerticalAlignment(EVRTA_TextCenter);
	ReadyText->SetWorldSize(40.f);
	ReadyText->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	ReadyText->SetText(FText::FromString(TEXT("[ 0 / 0 ]")));
	ReadyText->SetVisibility(false, true);
}

void APlatformRoot::BeginPlay()
{
	Super::BeginPlay();
	if (!HasAuthority())
	{
		SetActorTickEnabled(false);
	}
	if (HasAuthority())
	{
		SpawnCore();
	}
	PlatformMatrixComponent->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	PlatformMatrixComponent->InitMatrix();
}

void APlatformRoot::SetWaitMoveIn(bool bWaitMoveIn)
{
	// bool bWaitMoveIn =
	// 	CurState == EMASectorState::Wait || CurState == EMASectorState::EndBattle;
	PlatformMatrixComponent->SetMovedInPlatforms(bWaitMoveIn);
	if (HasAuthority())
	{
		bReadyTextVisible = bWaitMoveIn;
	}
	if (ReadyText)
	{
		ReadyText->SetVisibility(bWaitMoveIn, true);
	}
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

void APlatformRoot::SetReadyText(int32 ReadyCount, int32 TotalCount)
{
	if (HasAuthority())
	{
		ReplicatedReadyCounts = FIntPoint(ReadyCount, TotalCount);
	}
	if (!ReadyText) return;

	const FString NewText = FString::Printf(TEXT("[ %d / %d ]"), ReadyCount, TotalCount);
	ReadyText->SetText(FText::FromString(NewText));
}

void APlatformRoot::OnRep_ReadyCounts()
{
	if (!ReadyText) return;
	const FString NewText = FString::Printf(TEXT("[ %d / %d ]"), ReplicatedReadyCounts.X, ReplicatedReadyCounts.Y);
	ReadyText->SetText(FText::FromString(NewText));
}

void APlatformRoot::OnRep_ReadyTextVisible()
{
	if (!ReadyText) return;
	ReadyText->SetVisibility(bReadyTextVisible, true);
}

void APlatformRoot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlatformRoot, ReplicatedReadyCounts);
	DOREPLIFETIME(APlatformRoot, bReadyTextVisible);
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

	ACore* Core = GetWorld()->SpawnActor<ACore>(CoreClass, GetActorTransform(), Params);
	if (Core)
	{
		CoreInstance = Core;
		Core->AttachToComponent(
			Root,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale
		);
	}
	Core->SetActorRelativeLocation(FVector(0, 0, 100.f));
}
