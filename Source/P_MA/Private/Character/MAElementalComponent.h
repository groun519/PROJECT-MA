#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "MAElementalComponent.generated.h"

class AMACharacter;
class UMAAbilitySystemComponent;

UCLASS(ClassGroup=(Custom))
class P_MA_API UMAElementalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAElementalComponent();
	float GetTemperature() const { return CurrentTemperature; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BindToASC();
	void HandleTemperatureChanged(const FOnAttributeChangeData& Data);
	void RefreshTemperatureRecoveryEffect();
	bool IsTemperatureRecoveryActive() const;
	void ApplyTemperatureRecovery();
	void RemoveTemperatureRecovery();
	void RefreshTemperatureSlow();
	float CalculateTemperatureSlowMultiplier() const;
	bool IsTemperatureSlowActive() const;
	void ApplyTemperatureSlow(float SlowMultiplier);
	void RemoveTemperatureSlow();
	bool IsFrozenStatusActive() const;
	void RefreshFrozenStatus();
	void ApplyFrozenStatus();
	void RemoveFrozenStatus();

	UPROPERTY(EditDefaultsOnly, Category="Elemental|Frozen")
	float FrozenEnterTemperature = -100.f;

	UPROPERTY(EditDefaultsOnly, Category="Elemental|Frozen")
	float FrozenExitTemperature = -80.f;

	UPROPERTY(EditDefaultsOnly, Category="Elemental|Slow", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FrozenSlowMinMultiplier = 0.5f;

	UPROPERTY(Transient)
	TObjectPtr<AMACharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UMAAbilitySystemComponent> OwnerASC;

	float CurrentTemperature = 0.f;
	FActiveGameplayEffectHandle TemperatureRecoveryEffectHandle;
	FActiveGameplayEffectHandle TemperatureSlowEffectHandle;
	float CurrentTemperatureSlowMultiplier = 1.f;
	FActiveGameplayEffectHandle FrozenStatusEffectHandle;
};
