#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Action/MASkillAction_ProjectileBase.h"
#include "MASkillAction_ThrowWeaponProjectile.generated.h"

UCLASS(BlueprintType, EditInlineNew, DefaultToInstanced)
class P_MA_API UMASkillAction_ThrowWeaponProjectile : public UMASkillAction_ProjectileBase
{
	GENERATED_BODY()

protected:
	virtual bool PostSpawnProjectile(AMAProjectile& Projectile, AActor& AvatarActor, const FMASkillPayloadAccessor& Payloads) override;

private:
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FName ProjectileWeaponMeshComponentName = TEXT("WeaponVisual");
};
