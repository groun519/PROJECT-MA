// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PlayNiagara.generated.h"

class UNiagaraSystem;
/**
 * 
 */
UCLASS(Blueprintable)
class UAnimNotify_PlayNiagara : public UAnimNotify
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	TObjectPtr<UNiagaraSystem> NiagaraTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara", meta=(Categories="Module.Elemental"))
	TMap<FGameplayTag, TObjectPtr<UNiagaraSystem>> OverrideVFXMap;

	/** 위치/회전의 기준이 될 소켓 이름 (비어있으면 컴포넌트 루트) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	FName SocketName = NAME_None;

	/** 소켓 위치에서의 추가 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	FVector LocationOffset = FVector::ZeroVector;

	/** 소켓 회전에서의 추가 오프셋 (오일러 각도) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** 이펙트 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	FVector Scale = FVector(1.0f);

	/** true면 월드 공간에 스폰 (캐릭터 따라다니지 않음), false면 소켓에 부착 (캐릭터 따라다님) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	bool bSpawnInWorld = true; // 기본값은 월드 스폰

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Niagara")
	FName ColorParamName = TEXT("EffectColor");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Niagara")
	float BaseVFXLength = 0.f;
	
protected:
	// UAnimNotify 인터페이스 오버라이드
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

};
