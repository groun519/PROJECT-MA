#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "UObject/Object.h"
#include "MASkillRuntimeRegistry.generated.h"

class AActor;
class UAbilitySystemComponent;

USTRUCT()
struct FMASkillRegisteredGameplayEffect
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TWeakObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(Transient)
	FActiveGameplayEffectHandle Handle;
};

UCLASS()
class P_MA_API UMASkillRuntimeRegistry : public UObject
{
	GENERATED_BODY()

public:
	void Register(AActor* Actor);
	void Register(UAbilitySystemComponent* AbilitySystemComponent, FActiveGameplayEffectHandle Handle);
	void Cleanup();

private:
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AActor>> Actors;

	UPROPERTY(Transient)
	TArray<FMASkillRegisteredGameplayEffect> GameplayEffects;

	bool bCleanedUp = false;
};
