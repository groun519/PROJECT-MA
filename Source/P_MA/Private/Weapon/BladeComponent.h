// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "BladeComponent.generated.h"

class USkeletalMeshComponent;
class USceneComponent;
class AActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UBladeComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UBladeComponent();
	
	// Socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Socket")
	FName BaseSocketName = TEXT("WeaponBaseSocket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Socket")
	FName TipSocketName = TEXT("WeaponTipSocket");
	
	// Range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float Range = 100.f;
};
