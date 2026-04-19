#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "MAAbilitySystemGlobals.generated.h"

UCLASS()
class UMAAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
