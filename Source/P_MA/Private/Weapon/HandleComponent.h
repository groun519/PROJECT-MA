// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "HandleComponent.generated.h"

class USkeletalMeshComponent;
class USceneComponent;
class AActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UHandleComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UHandleComponent();

	// Socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Socket")
	FName BladeAttachSocketName = TEXT("WeaponBladeSocket");
};
