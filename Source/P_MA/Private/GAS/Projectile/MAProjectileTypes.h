#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Event/MASkillEventTypes.h"
#include "MAProjectileTypes.generated.h"

class AActor;
class UNiagaraSystem;
class UMASkillAbility;
class UMASkillModuleInstance;

USTRUCT()
struct P_MA_API FMAProjectileTargetSettings
{
	GENERATED_BODY()

	TWeakObjectPtr<AActor> TargetActor;
	bool bHitOnlyTarget = false;
};

USTRUCT()
struct P_MA_API FMAProjectileElementalSettings
{
	GENERATED_BODY()

	bool bHasElementalData = false;
	FLinearColor ElementalColor = FLinearColor::White;
	TObjectPtr<UNiagaraSystem> MainVFX = nullptr;
	TObjectPtr<UNiagaraSystem> TrailVFX = nullptr;
};

USTRUCT(BlueprintType)
struct P_MA_API FMAProjectileElementalVisualSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Elemental")
	bool bUseElementalVFX = true;

	UPROPERTY(EditDefaultsOnly, Category="Elemental")
	bool bUseElementalColor = true;
};

USTRUCT(BlueprintType)
struct P_MA_API FMAProjectileContinuousHitSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Sweep", meta=(ClampMin="0.01", UIMin="0.01"))
	float TickInterval = 0.08f;

	UPROPERTY(EditDefaultsOnly, Category="Sweep", meta=(ClampMin="0.0", UIMin="0.0"))
	float MinSweepDistance = 10.f;

	UPROPERTY(EditDefaultsOnly, Category="Sweep", meta=(ClampMin="1.0", UIMin="1.0"))
	float MaxSweepSegmentLength = 150.f;

	UPROPERTY(EditDefaultsOnly, Category="Sweep", meta=(ClampMin="1", UIMin="1"))
	int32 MaxSweepSubsteps = 8;
};

USTRUCT()
struct P_MA_API FMAProjectileParams
{
	GENERATED_BODY()

	FResolvedSkillDamage ResolvedDamage;
	float ProjectileRadiusMultiplier = 1.f;
	int32 MaxHitCount = 1;

	FMAProjectileTargetSettings TargetSettings;
	FMAProjectileElementalSettings ElementalSettings;
	FMAProjectileContinuousHitSettings ContinuousHitSettings;
	TWeakObjectPtr<UMASkillAbility> EventExecutorAbility;
	FMASkillScopes EventScopes;
};
