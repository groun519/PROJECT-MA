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
	
	float CenterDistance =
		(PlatformRoot->GetActorLocation() - NextSector->GetActorLocation()).Length();

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
		Sectors[LastSectorIndex]->GetSeed
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



