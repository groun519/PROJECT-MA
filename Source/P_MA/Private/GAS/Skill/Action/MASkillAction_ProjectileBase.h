#pragma once

#include "CoreMinimal.h"
#include "GAS/Projectile/MAProjectileTypes.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_ProjectileBase.generated.h"

class AMAProjectileBase;
struct FMASkillPayloadAccess;

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
	TSubclassOf<AMAProjectileBase> ProjectileClass;

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

	UPROPERTY(EditDefaultsOnly, Category="Projectile", meta=(ClampMin="0", UIMin="0", ToolTip="Maximum unique targets this projectile can hit. Set to 0 for unlimited hits."))
	int32 MaxHitCount = 1;

	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	FMAProjectileContinuousHitSettings ContinuousHitSettings;
};

UCLASS(Abstract, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction_ProjectileBase : public UMASkillAction
{
	GENERATED_BODY()

public:
	UMASkillAction_ProjectileBase() { SupportedModuleTypes = EMASkillModuleType::Module | EMASkillModuleType::Sub; }

	virtual void Execute(
		AActor& Owner,
		UMASkillAbility* Ability,
		const FMASkillEvent& Event,
		const FMASkillScopes* Scopes) override;

protected:
	virtual bool PostSpawnProjectile(AMAProjectileBase& Projectile, AActor& AvatarActor, const FMASkillPayloadAccess& Payloads);

	UPROPERTY(EditDefaultsOnly, Category="Action")
	FMASkillActionConfig_SpawnProjectile Config;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;
};
