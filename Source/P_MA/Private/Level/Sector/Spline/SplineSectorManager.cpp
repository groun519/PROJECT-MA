// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineSectorManager.h"
#include "DrawDebugHelpers.h"

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
	PlatformLoc.Y = 0; PlatformLoc.Z = 0;
	FVector FinalSectorLoc = Sectors[Sectors.Num() - 1]->GetActorLocation();
	FinalSectorLoc.Y = 0; FinalSectorLoc.Z = 0;
	
	float CenterDistance = (PlatformLoc - FinalSectorLoc).Length();

	UE_LOG(LogTemp, Display, TEXT("CenterDistance: %f"), CenterDistance);
	DrawDebugSphere(GetWorld(), FinalSectorLoc, 12.f, 12.f,FColor::Red, true, 1, 0, 0);
	
	if (CenterDistance < 100.f)
		return true;
	return false;
}

void ASplineSectorManager::GoBackToFirstSector()
{
	int32 LastSectorIndex = Sectors.Num() - 1;
	UE_LOG(LogTemp, Display, TEXT("LastSectorIndex: %d"), LastSectorIndex);

	Sectors[0]->SetSectorSeed(Sectors[LastSectorIndex]->GetSectorSeed());
	
	FVector FirstSectorLoc = Sectors[0]->GetActorLocation();
	FVector LastSectorLoc = Sectors[LastSectorIndex]->GetActorLocation();
	FVector PlatformLoc = PlatformRoot->GetActorLocation();

	FVector Offset = PlatformLoc - LastSectorLoc;
	FirstSectorLoc += Offset;
	
	FirstSectorLoc.Z = 50;
	PlatformRoot->SetActorLocation(FirstSectorLoc);
}

int32 ASplineSectorManager::GetNextSectorIndex(int32 CurSectorIndex)
{
	int32 LastSectorIndex = Sectors.Num() - 1;
	return CurSectorIndex == LastSectorIndex ? 0 : CurSectorIndex + 1;
}

ASplineSectorManager* ASplineSectorManager::FindSplineSectorManager(UWorld* World)
{
	AActor* Found = UGameplayStatics::GetActorOfClass(World, ASplineSectorManager::StaticClass());
	ASplineSectorManager* SSM = Cast<ASplineSectorManager>(Found);
	return SSM;
}

void ASplineSectorManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if (IsClosePreSectorZeroVector())
	// {
	// 	GoBackToFirstSector();
	// }
}



