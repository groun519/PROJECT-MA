#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LoadoutWeaponData.generated.h"

class USkeletalMesh;
class UTexture2D;
class UMASkillModule;

USTRUCT(BlueprintType)
struct FLoadoutWeaponDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSoftObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSoftObjectPtr<USkeletalMesh> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	FTransform WeaponOffset = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSoftObjectPtr<UMASkillModule> AttackSkillModule;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSoftObjectPtr<UMASkillModule> PassiveSkillModule;
};
