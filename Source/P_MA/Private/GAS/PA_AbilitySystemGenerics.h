#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PA_AbilitySystemGenerics.generated.h"

class UGameplayEffect;
class UGameplayAbility;

/**
 * 
 */
UCLASS()
class UPA_AbilitySystemGenerics : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	FORCEINLINE TSubclassOf<UGameplayEffect> GetFullStatEffect() const { return FullStatEffect; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetReviveStatEffect() const { return ReviveStatEffect; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDeathEffect() const { return DeathEffect; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetDamageEffect() const { return DamageGEClass; }
	FORCEINLINE TSubclassOf<UGameplayEffect> GetCooldownEffect() const { return CooldownGEClass; }
	FORCEINLINE TSubclassOf<UGameplayAbility> GetDeadAbility() const { return DeadAbility; }


private:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> FullStatEffect;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> ReviveStatEffect;
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Effects")
	TSubclassOf<UGameplayEffect> DeathEffect;
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TSubclassOf<UGameplayEffect> DamageGEClass;
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Effects")
	TSubclassOf<UGameplayEffect> CooldownGEClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Ability")
	TSubclassOf<UGameplayAbility> DeadAbility;
};
