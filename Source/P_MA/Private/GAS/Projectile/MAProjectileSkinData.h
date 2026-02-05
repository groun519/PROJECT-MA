// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "MAProjectileSkinData.generated.h"


class UNiagaraSystem;
class AMAProjectile;

USTRUCT(BlueprintType)
struct FProjectileSkinInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAProjectile> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> ProjectileVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(Categories="GameplayCue.Hit"))
	FGameplayTag HitCueTag;
};
/**
 * 
 */
UCLASS()
class UMAProjectileSkinData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FProjectileSkinInfo DefaultSkin;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TMap<FGameplayTag, FProjectileSkinInfo> SkinOverrides;

	FProjectileSkinInfo GetSkinForTag(FGameplayTag ElementTag) const
	{
		if (ElementTag.IsValid() && SkinOverrides.Contains(ElementTag))
		{
			return SkinOverrides[ElementTag];
		}
		return DefaultSkin;
	}
};
