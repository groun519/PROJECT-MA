#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MACurrencyComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UMACurrencyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMACurrencyComponent();

	float GetCoin() const;
	bool HasCoin(float Amount) const;
	bool TrySpendCoin(float Amount);
	void AddCoin(float Amount);
};
