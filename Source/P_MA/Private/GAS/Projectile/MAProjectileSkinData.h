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
	/**속성 별로 투사체에 적용시킬 VFX 이펙트*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UNiagaraSystem> ProjectileVFX;
	/**속성 별 타격 시 발생시킬 CueTag*/
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
	/**투사체 블루프린트*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AMAProjectile> ProjectileClass;
	/**ProjectileClass에 적용시킬 값들*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FProjectileSkinInfo DefaultSkin;
	/**속성 태그 별 적용시킬 값들*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(Categories="Module.Elemental"))
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
