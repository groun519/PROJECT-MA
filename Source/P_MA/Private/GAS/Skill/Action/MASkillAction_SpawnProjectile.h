#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Projectile/MAProjectile.h"
#include "GameplayTagContainer.h"
#include "MASkillAction_SpawnProjectile.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FMASkillActionConfig_SpawnProjectile
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category="Projectile")
	TSubclassOf<AMAProjectile> ProjectileClass;

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
	virtual void Execute(UMASkillAbility& OwnerAbility, const FGameplayEventData& Payload) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Action")
	FMASkillActionConfig_SpawnProjectile Config;

	UPROPERTY(EditDefaultsOnly, Category="Damage", meta=(Categories="Damage"))
	FGameplayTag DamagePayloadTag;
};
