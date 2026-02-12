#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "CoinDrop.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UCoinDrop : public UActorComponent
{
	GENERATED_BODY()

public:
	UCoinDrop();

	UPROPERTY(EditAnywhere, Category="FX")
	UNiagaraSystem* CoinAbsorbFX;

	UPROPERTY(EditAnywhere, Category="FX")
	float AbsorbDelay = 1.5f;	

	void SpawnCoinFX();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	UNiagaraComponent* ActiveCoinFX;

	bool bCanAbsorb = false;
	FTimerHandle AbsorbDelayTimer;

	void EnableAbsorb();
};
