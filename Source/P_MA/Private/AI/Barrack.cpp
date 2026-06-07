#include "AI/Barrack.h"

#include "AI/Golem/Monster.h"
#include "GameFramework/PlayerStart.h"

ABarrack::ABarrack()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABarrack::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(SpawnIntervalTimerHandle, this, &ABarrack::SpawnNewGroup, GroupSpawnInterval, true);
	}
}

void ABarrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

const APlayerStart* ABarrack::GetNextSpawnSpot()
{
	if (SpawnSpots.Num() == 0) return nullptr;
	
	++NextSpawnSpotIndex;

	if (NextSpawnSpotIndex >= SpawnSpots.Num())
		NextSpawnSpotIndex = 0;

	return SpawnSpots[NextSpawnSpotIndex];
}

void ABarrack::SpawnNewGroup()
{
	int i = MonsterPerGroup;

	while (i > 0)
	{
		FTransform SpawnTransfrom = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnSpot())
		{
			SpawnTransfrom = NextSpawnSpot->GetActorTransform();
		}
		
		AMonster* NextAvaliableMonster = GetNextAvaliableMonster();
		if (!NextAvaliableMonster)
			break;

		NextAvaliableMonster->SetActorTransform(SpawnTransfrom);
		NextAvaliableMonster->Activate();
		--i;
	}

	SpawnNewMinions(i);
}

void ABarrack::SpawnNewMinions(int Amt)
{
	for (int i = 0; i < Amt; i++)
	{
		FTransform SpawnTransfrom = GetActorTransform();
		if (const APlayerStart* NextSpawnSpot = GetNextSpawnSpot())
		{
			SpawnTransfrom = NextSpawnSpot->GetActorTransform();
		}

		AMonster* NewMonster = GetWorld()->SpawnActorDeferred<AMonster>(MonsterClass, SpawnTransfrom, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		NewMonster->SetGenericTeamId(BarrackTeamId);
		NewMonster->FinishSpawning(SpawnTransfrom);
		NewMonster->SetGoal(Goal);
		MonsterPool.Add(NewMonster);
	}
}

AMonster* ABarrack::GetNextAvaliableMonster() const
{
	for(AMonster* Minion : MonsterPool)
	{
		if (!Minion->IsActive())
		{
			return Minion;
		}
	}
	return nullptr;
}
