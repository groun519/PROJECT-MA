// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Loadout/LoadoutComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"

ULoadoutComponent::ULoadoutComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULoadoutComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULoadoutComponent, MaterialParamValue);
}

void ULoadoutComponent::InitializeMaterial(USkeletalMeshComponent* InMesh)
{
	TargetMesh = InMesh;
	if (!TargetMesh)
	{
		return;
	}

	DynMat = TargetMesh->CreateAndSetMaterialInstanceDynamic(0);
	if (DynMat)
	{
		ApplyMaterialParam(BaseMaterialParam);
	}
}

void ULoadoutComponent::SetMaterialParams(const FMaterialParamData& BodyData, const FMaterialParamData& EyeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		Server_SetMaterialParams(BodyData, EyeData);
		return;
	}

	MaterialParamValue.BodyData = BodyData;
	MaterialParamValue.EyeData = EyeData;
	ApplyMaterialParam(MaterialParamValue);
}

void ULoadoutComponent::Server_SetMaterialParams_Implementation(const FMaterialParamData& BodyData,
	const FMaterialParamData& EyeData)
{
	SetMaterialParams(BodyData, EyeData);
}

void ULoadoutComponent::OnRep_MaterialParam()
{
	ApplyMaterialParam(MaterialParamValue);
}

void ULoadoutComponent::ApplyMaterialParam(const FMaterialParamDataPair& Params)
{
	if (!DynMat && TargetMesh)
	{
		DynMat = TargetMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (!DynMat)
	{
		return;
	}

	DynMat->SetVectorParameterValue("Body_Color", Params.BodyData.Color);
	DynMat->SetScalarParameterValue("Body_Emissive", Params.BodyData.Emissive);
	DynMat->SetVectorParameterValue("Eye_Color", Params.EyeData.Color);
	DynMat->SetScalarParameterValue("Eye_Emissive", Params.EyeData.Emissive);
}
