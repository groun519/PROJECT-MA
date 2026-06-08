#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "MAElementalComponent.generated.h"

class AMACharacter;
class UMAAbilitySystemComponent;
class UMAElementalConfigData;
class UMaterialInstanceDynamic;

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
	const UMAElementalConfigData* GetElementalConfigData() const;
	void BindToASC();
	void HandleTemperatureChanged(const FOnAttributeChangeData& Data);
	void RefreshTemperatureRecoveryEffect();
	void RefreshTemperatureOverlay();
	float CalculateTemperatureOverlayAlpha() const;
	bool IsTemperatureRecoveryActive() const;
	void ApplyTemperatureRecovery();
	void RemoveTemperatureRecovery();
	void RefreshTemperatureSlow();
	float CalculateTemperatureSlowMultiplier() const;
	bool IsTemperatureSlowActive() const;
	void ApplyTemperatureSlow(float SlowMultiplier);
	void RemoveTemperatureSlow();
	void RefreshBurnDamage(const FGameplayEffectContextHandle& SourceContext = FGameplayEffectContextHandle());
	float GetMaxBurnDamagePerTick() const;
	bool IsBurnDamageActive() const;
	void ApplyBurnDamage(const FGameplayEffectContextHandle& SourceContext);
	void RemoveBurnDamage();
	bool IsFrozenStatusActive() const;
	void RefreshFrozenStatus();
	void ApplyFrozenStatus();
	void RemoveFrozenStatus();

	UPROPERTY(EditDefaultsOnly, Category="Elemental")
	TObjectPtr<UMAElementalConfigData> OverrideElementalConfigData;

	UPROPERTY(Transient)
	TObjectPtr<AMACharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UMAAbilitySystemComponent> OwnerASC;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> TemperatureOverlayMID;

	float CurrentTemperature = 0.f;
	FActiveGameplayEffectHandle TemperatureRecoveryEffectHandle;
	FActiveGameplayEffectHandle TemperatureSlowEffectHandle;
	float CurrentTemperatureSlowMultiplier = 1.f;
	FActiveGameplayEffectHandle BurnDamageEffectHandle;
	FActiveGameplayEffectHandle FrozenStatusEffectHandle;
};
