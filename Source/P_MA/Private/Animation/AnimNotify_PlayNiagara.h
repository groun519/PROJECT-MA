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
	TObjectPtr<UNiagaraSystem> VFXToSpawn;

	/** 위치/회전의 기준이 될 소켓 이름 (비어있으면 컴포넌트 루트) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara")
	FName SocketName;

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

	/** bSpawnInWorld가 false일 때만 적용됨: 컴포넌트가 파괴될 때 자동으로 파티클도 제거할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara", meta = (EditCondition = "!bSpawnInWorld"))
	bool bAutoDestroy = true;

	/** bSpawnInWorld가 false일 때만 적용됨: 이미 부착된 같은 종류의 파티클이 있다면 제거할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Niagara", meta = (EditCondition = "!bSpawnInWorld"))
	bool bAbsoluteScale = false; // 기본값 false (Scale은 상대 크기)

protected:
	// UAnimNotify 인터페이스 오버라이드
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

};
