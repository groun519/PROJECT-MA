// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineSectorManager.h"

#include "Kismet/GameplayStatics.h"


ASplineSectorManager::ASplineSectorManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASplineSectorManager::BeginPlay()
{
	Super::BeginPlay();

	FVector SpawnLoc = GetActorLocation();
	ASplineSector* NewPreSector =
			GetWorld()->SpawnActor<ASplineSector>(
				SectorClass,
				SpawnLoc,
				FRotator::ZeroRotator
			);
	PreSector = NewPreSector;

	SpawnLoc.X += (NewPreSector->GetSectorBound().X * 100);
	ASplineSector* NewNextSector =
			GetWorld()->SpawnActor<ASplineSector>(
				SectorClass,
				SpawnLoc,
				FRotator::ZeroRotator
			);
	NextSector = NewNextSector;

	PlatformRoot = Cast<APlatformRoot>(
	UGameplayStatics::GetActorOfClass(GetWorld(), APlatformRoot::StaticClass())
);
}

void ASplineSectorManager::SwapNextSector()
{
	if (!SectorClass) return;

	ASplineSector* TempPreSector = PreSector;

	PreSector->AddActorWorldOffset(
		PreSector->GetActorForwardVector() * (PreSector->GetSectorBound().X * 100 * 2)
		);
	PreSector->SetRandomSeed();

	PreSector = NextSector;
	NextSector = TempPreSector;

	TryRebaseWorld();
}

void ASplineSectorManager::TryRebaseWorld()
{
	if (!PreSector) return;

	const FVector Pos = PreSector->GetActorLocation();

	const FIntVector NewOrigin(
		(int32)Pos.X,
		(int32)Pos.Y,
		0
	);

	GetWorld()->SetNewWorldOrigin(NewOrigin);
}

bool ASplineSectorManager::IsClosePreSectorZeroVector()
{
	if (!PlatformRoot) return false;
	
	float CenterDistance =
		(PlatformRoot->GetActorLocation() - NextSector->GetActorLocation()).Length();

	UE_LOG(LogTemp, Display, TEXT("CenterDistance: %f"), CenterDistance);
	
	if (CenterDistance < 100.f)
		return true;
	return false;
}

void ASplineSectorManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsClosePreSectorZeroVector())
	{
		SwapNextSector();
	}
}



