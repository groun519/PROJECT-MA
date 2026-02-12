// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LoadoutWeaponData.generated.h"

class UGameplayAbility;
class USkeletalMesh;
class UTexture2D;

USTRUCT(BlueprintType)
struct FLoadoutWeaponDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	FText WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSoftObjectPtr<UTexture2D> IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSoftObjectPtr<USkeletalMesh> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	FTransform WeaponOffset = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loadout|Weapon")
	TSubclassOf<UGameplayAbility> AttackAbility;
};
