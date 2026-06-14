#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectileTypes.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_SpawnProjectile.generated.h"

class AMAProjectile;

UENUM(BlueprintType)
enum class EMASkillProjectileStartObjectSource : uint8
{
	Self,
	ObjectPayload
};

UENUM(BlueprintType)
enum class EMASkillProjectileDirectionSource : uint8
{
	Forward,
	Self,
	ObjectPayload
};

UENUM(BlueprintType)
enum class EMASkillProjectileTrackingTargetSource : uint8
{
	Self,
	ObjectPayload
};

USTRUCT(BlueprintType)
struct FMASkillProjectileTrackingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Tracking")
	EMASkillProjectileTrackingTargetSource TargetSource = EMASkillProjectileTrackingTargetSource::Self;

	UPROPERTY(EditDefaultsOnly, Category="Tracking", meta=(Categories="Data", EditCondition="TargetSource == EMASkillProjectileTrackingTargetSource::ObjectPayload", EditConditionHides))
	FGameplayTag TargetObjectPayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Tracking")
	bool bHitOnlyTarget = false;
};

USTRUCT(BlueprintType)
struct FMASkillActionConfig_SpawnProjectile
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<AMAProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	EMASkillProjectileStartObjectSource StartObjectSource = EMASkillProjectileStartObjectSource::Self;

	UPROPERTY(EditDefaultsOnly, Category="Projectile", meta=(Categories="Data", EditCondition="StartObjectSource == EMASkillProjectileStartObjectSource::ObjectPayload", EditConditionHides))
	FGameplayTag StartObjectPayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	EMASkillProjectileDirectionSource DirectionSource = EMASkillProjectileDirectionSource::Forward;

	UPROPERTY(EditDefaultsOnly, Category="Projectile", meta=(Categories="Data", EditCondition="DirectionSource == EMASkillProjectileDirectionSource::ObjectPayload", EditConditionHides))
	FGameplayTag DirectionObjectPayloadTag;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	bool bUseTargetTracking = false;

	UPROPERTY(EditDefaultsOnly, Category="Projectile", meta=(EditCondition="bUseTargetTracking", EditConditionHides))
	FMASkillProjectileTrackingConfig TargetTracking;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	FName SpawnSocketName = TEXT("WeaponHandSocket");

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	bool bIsPenetrating = false;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	FMAProjectileContinuousHitSettings ContinuousHitSettings;
};

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction_SpawnProjectile : public UMASkillAction
{
	GENERATED_BODY()

public:
	virtual void Execute(UMASkillAbility& OwnerAbility, const FMASkillEvent& Event, const FMASkillScopes& Scopes) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Action")
	FMASkillActionConfig_SpawnProjectile Config;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;
};
