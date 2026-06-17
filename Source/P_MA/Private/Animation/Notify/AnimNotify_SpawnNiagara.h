#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SpawnNiagara.generated.h"

class UNiagaraSystem;

UENUM()
enum class EMANiagaraAttachTarget : uint8
{
	OwnerMesh,
	WeaponMesh,
};

UCLASS()
class UAnimNotify_SpawnNiagara : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	UPROPERTY(EditAnywhere, Category = "Niagara")
	TObjectPtr<UNiagaraSystem> NiagaraTemplate = nullptr;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	bool bSpawnInWorld = true;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	EMANiagaraAttachTarget AttachTarget = EMANiagaraAttachTarget::OwnerMesh;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	FName SocketName = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	FRotator RotationOffset = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	FVector Scale = FVector(1.f);

	UPROPERTY(EditAnywhere, Category = "Niagara")
	bool bApplySkillAreaScale = true;

	UPROPERTY(EditAnywhere, Category = "Niagara")
	bool bApplyElementalColor = true;

	UPROPERTY(EditAnywhere, Category = "Niagara", meta=(EditCondition="bApplyElementalColor", EditConditionHides))
	FName ColorParamName = TEXT("User.BaseColor");
};
