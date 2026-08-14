#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
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
	void HandleDeathChanged(FGameplayTag DeadTag, int32 TagCount);
	void HandleTemperatureChanged(const FOnAttributeChangeData& Data);
	void HandleTemperatureImmunityChanged(FGameplayTag ImmunityTag, int32 TagCount);
	void RefreshTemperatureRecoveryEffect();
	void DelayTemperatureRecovery();
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
	bool IsBurnDamageActive() const;
	FActiveGameplayEffectHandle ApplyBurnDamageEffect(
		const FGameplayEffectContextHandle& SourceContext,
		float MaxDamagePerTick,
		float TickInterval) const;
	void RemoveBurnDamage();
	void TriggerOverheatExplosion(const FGameplayEffectContextHandle& SourceContext);
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
	FTimerHandle TemperatureRecoveryDelayTimerHandle;
	FActiveGameplayEffectHandle TemperatureRecoveryEffectHandle;
	FActiveGameplayEffectHandle TemperatureSlowEffectHandle;
	float CurrentTemperatureSlowMultiplier = 1.f;
	FActiveGameplayEffectHandle BurnDamageEffectHandle;
	bool bOverheated = false;
	FActiveGameplayEffectHandle FrozenStatusEffectHandle;
};
