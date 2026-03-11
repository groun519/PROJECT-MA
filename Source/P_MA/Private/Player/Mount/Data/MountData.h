// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MountData.generated.h"

class UAnimInstance;
class UAnimSequence;
class UMaterialInterface;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct FMountDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mount")
	TSoftObjectPtr<UMaterialInterface> IconMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mount")
	TSoftObjectPtr<USkeletalMesh> MountMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mount")
	TSubclassOf<UAnimInstance> MountAnimClass;

	// Player character sequence used while riding this mount.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mount")
	TSoftObjectPtr<UAnimSequence> RiderPose;
};
