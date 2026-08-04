#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "MAElementalConfigData.generated.h"

UCLASS(BlueprintType)
class P_MA_API UMAElementalConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	UMAElementalConfigData();

	UPROPERTY(EditDefaultsOnly, Category="Frozen")
	float FrozenEnterTemperature = -100.f;

	UPROPERTY(EditDefaultsOnly, Category="Frozen")
	float FrozenExitTemperature = -80.f;

	UPROPERTY(EditDefaultsOnly, Category="Frozen", meta=(ClampMin="0.0", ClampMax="1.0"))
	float FrozenSlowMinMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Recovery", meta=(ClampMin="0.0"))
	float TemperatureRecoveryDelay = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Recovery", meta=(ClampMin="0.01"))
	float TemperatureRecoveryTickInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category="Recovery", meta=(ClampMin="0.0", ClampMax="1.0"))
	float TemperatureRecoveryRatioPerTick = 0.01f;

	UPROPERTY(EditDefaultsOnly, Category="Recovery", meta=(ClampMin="0.0"))
	float TemperatureRecoveryAmountPerTick = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category="Burn", meta=(ClampMin="0.0"))
	float MaxBurnDamagePerTick = 5.f;

	UPROPERTY(EditDefaultsOnly, Category="Burn", meta=(ClampMin="0.01"))
	float BurnTickInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Burn|GameplayCue", meta=(Categories="GameplayCue.Hit"))
	FGameplayTagContainer BurnGameplayCueTags;

	UPROPERTY(EditDefaultsOnly, Category="Burn|Overheat")
	float OverheatEnterTemperature = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Burn|Overheat")
	float OverheatExitTemperature = 80.f;

	UPROPERTY(EditDefaultsOnly, Category="Burn|Overheat", meta=(ClampMin="0.0"))
	float OverheatedBurnDamagePerTick = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="Burn|Overheat", meta=(ClampMin="0.01"))
	float OverheatedBurnTickInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category="Burn|Overheat", meta=(ClampMin="0.0"))
	float OverheatExplosionRadius = 300.f;

	UPROPERTY(EditDefaultsOnly, Category="Burn|Overheat")
	TArray<FMASkillDamageConfig> OverheatExplosionDamages;
};
