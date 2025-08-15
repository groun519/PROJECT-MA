// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "WeaponComponent.generated.h"

class USkeletalMeshComponent;
class USceneComponent;
class AActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UWeaponComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();
	
	// Socket
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Socket")
	FName HandSocketName = TEXT("WeaponHandSocket");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Socket")
	FName BaseSocketName = TEXT("WeaponBaseSocket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Socket")
	FName TipSocketName = TEXT("WeaponTipSocket");
	
	// Range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float Range = 100.f;

	// Getter
	UFUNCTION(BlueprintCallable, Category="Blade")
	FVector GetBladeBaseSocketLocation();

	UFUNCTION(BlueprintCallable, Category="Blade")
	FVector GetBladeTipSocketLocation();
};
