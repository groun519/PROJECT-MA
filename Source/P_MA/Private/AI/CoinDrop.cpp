#include "CoinDrop.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UCoinDrop::UCoinDrop()
{
	PrimaryComponentTick.bCanEverTick = true;
	ActiveCoinFX = nullptr;
	bCanAbsorb = false;
}

void UCoinDrop::SpawnCoinFX()
{
	if (!CoinAbsorbFX) return;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	APlayerController* PC = Owner->GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;

	ActiveCoinFX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		Owner->GetWorld(),
		CoinAbsorbFX,
		Owner->GetActorLocation()
	);

	if (ActiveCoinFX)
	{
		bCanAbsorb = false;

		GetWorld()->GetTimerManager().SetTimer(
			AbsorbDelayTimer,
			this,
			&UCoinDrop::EnableAbsorb,
			AbsorbDelay,
			false
		);
	}
}

void UCoinDrop::EnableAbsorb()
{
	bCanAbsorb = true;
}

void UCoinDrop::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ActiveCoinFX) return;
	if (!bCanAbsorb) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->GetPawn()) return;

	ActiveCoinFX->SetVectorParameter(
		TEXT("User.TargetLocation"),
		PC->GetPawn()->GetActorLocation()
	);
}
