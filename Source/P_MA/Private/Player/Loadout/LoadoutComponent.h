// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/Loadout/LoadoutColorTypes.h"
#include "LoadoutComponent.generated.h"

class USkeletalMeshComponent;
class UMaterialInstanceDynamic;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API ULoadoutComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULoadoutComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeMaterial(USkeletalMeshComponent* InMesh);

	void SetMaterialParams(const FMaterialParamData& BodyData, const FMaterialParamData& EyeData);

	UFUNCTION(Server, Reliable)
	void Server_SetMaterialParams(const FMaterialParamData& BodyData, const FMaterialParamData& EyeData);

	void ApplyMaterialParamsLocal(const FMaterialParamDataPair& Params);
	void ApplyMaterialParam(const FMaterialParamDataPair& Params, float SaturationScale = 1.f);

	const FMaterialParamDataPair& GetBaseMaterialParam() const { return BaseMaterialParam; }
	const FMaterialParamDataPair& GetMaterialParamValue() const { return MaterialParamValue; }

	// TODO: Save/Load hooks for loadout data.

private:
	static FLinearColor ApplySaturationScale(const FLinearColor& InColor, float SaturationScale);

	UFUNCTION()
	void OnRep_MaterialParam();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Loadout|Material", meta=(AllowPrivateAccess="true"))
	FMaterialParamDataPair BaseMaterialParam;

	UPROPERTY(ReplicatedUsing=OnRep_MaterialParam)
	FMaterialParamDataPair MaterialParamValue;

	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> TargetMesh;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> DynMats;
};
