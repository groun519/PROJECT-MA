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

	PlatformRoot = Cast<APlatformRoot>(
	UGameplayStatics::GetActorOfClass(GetWorld(), APlatformRoot::StaticClass())
	);	
}

bool ASplineSectorManager::IsClosePreSectorZeroVector()
{
	if (!PlatformRoot) return false;

	FVector PlatformLoc = PlatformRoot->GetActorLocation();
	PlatformLoc.X = 0;
	FVector FinalSectorLoc = Sectors[Sectors.Num() - 1]->GetActorLocation();
	FinalSectorLoc.X = 0;
	
	float CenterDistance = (PlatformLoc - FinalSectorLoc).Length();

	UE_LOG(LogTemp, Display, TEXT("CenterDistance: %f"), CenterDistance);
	
	if (CenterDistance < 100.f)
		return true;
	return false;
}

void ASplineSectorManager::GoBackToFirstSector()
{
	int32 LastSectorIndex = Sectors.Num() - 1;
	if (PreSectorIndex == LastSectorIndex)
	{
		Sectors[0]->SetSeed(Sectors[LastSectorIndex]->GetSectorSeed());
		PlatformRoot->SetActorLocation(Sectors[0]->GetActorLocation());
	}
}

void ASplineSectorManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsClosePreSectorZeroVector())
	{
		GoBackToFirstSector();
	}
}



