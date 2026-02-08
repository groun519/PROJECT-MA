// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/Loadout/LoadoutComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"

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

	DynMats.Reset();
	const int32 MaterialCount = TargetMesh->GetNumMaterials();
	for (int32 Index = 0; Index < MaterialCount; ++Index)
	{
		if (UMaterialInstanceDynamic* DynMat = TargetMesh->CreateAndSetMaterialInstanceDynamic(Index))
		{
			DynMats.Add(DynMat);
		}
	}

	if (DynMats.Num() > 0)
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

void ULoadoutComponent::ApplyMaterialParamsLocal(const FMaterialParamDataPair& Params)
{
	MaterialParamValue = Params;
	ApplyMaterialParam(MaterialParamValue);
}

void ULoadoutComponent::OnRep_MaterialParam()
{
	ApplyMaterialParam(MaterialParamValue);
}

void ULoadoutComponent::ApplyMaterialParam(const FMaterialParamDataPair& Params)
{
	UE_LOG(LogTemp, Warning, TEXT("Loadout: ApplyMaterialParam Owner=%s TargetMesh=%s Mats=%d Body=%s Eye=%s"),
		*GetNameSafe(GetOwner()),
		*GetNameSafe(TargetMesh),
		DynMats.Num(),
		*Params.BodyData.Color.ToString(),
		*Params.EyeData.Color.ToString());

	if (!TargetMesh)
	{
		if (const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
		{
			TargetMesh = OwnerCharacter->GetMesh();
		}
	}

	if (DynMats.Num() == 0 && TargetMesh)
	{
		const int32 MaterialCount = TargetMesh->GetNumMaterials();
		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			if (UMaterialInstanceDynamic* DynMat = TargetMesh->CreateAndSetMaterialInstanceDynamic(Index))
			{
				DynMats.Add(DynMat);
			}
		}
	}
	if (DynMats.Num() == 0)
	{
		return;
	}

	for (UMaterialInstanceDynamic* DynMat : DynMats)
	{
		if (!DynMat)
		{
			continue;
		}
		DynMat->SetVectorParameterValue("Body_Color", Params.BodyData.Color);
		DynMat->SetScalarParameterValue("Body_Emissive", Params.BodyData.Emissive);
		DynMat->SetVectorParameterValue("Eye_Color", Params.EyeData.Color);
		DynMat->SetScalarParameterValue("Eye_Emissive", Params.EyeData.Emissive);
	}
}
