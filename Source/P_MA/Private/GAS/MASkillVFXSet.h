// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "MASkillVFXSet.generated.h"

class UNiagaraSystem;
class UGameplayEffect;

/**
 * 스킬에서 스폰할 VFX 정보 구조체
 */
USTRUCT(BlueprintType)
struct F_SkillVFX_Info
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	TObjectPtr<UNiagaraSystem> DefaultVFX;

	/**재정의할 VFX
	 * FName = 속성 태그의 마지막 (Fire / Ice)
	 * 색상 변경 필요없는 경우에만 설정
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (EditCondition = "!bUseElementColor"))
	TMap<FName, TObjectPtr<UNiagaraSystem>> ElementVFXOverride;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	bool bUseElementColor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FName SocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	FVector Scale = FVector(1.0f);

	/** true면 월드 공간에 스폰 (캐릭터 따라다니지 않음), false면 소켓에 부착 (캐릭터 따라다님) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	bool bSpawnInWorld = true;

	//컴포넌트가 파괴될 때 자동으로 파티클도 제거할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX", meta = (EditCondition = "!bSpawnInWorld"))
	bool bAutoDestroy = true;
};

USTRUCT(BlueprintType)
struct F_ElementInfoRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element")
	FLinearColor ElementColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Element")
	float ElementalDamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Element")
	TSubclassOf<UGameplayEffect> ElementEffect;
};
/**
 * 
 */
UCLASS()
class UMASkillVFXSet : public UDataAsset
{
	GENERATED_BODY()

public:
	/**
	 * 해당 행동 모듈에서 사용할 모든 VFX 정보 저장 맵
	 * Key = 몽타주에서 받을 태그
	 * Value = VFX의 모든 정보
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, F_SkillVFX_Info> VFXDataMap;
};
