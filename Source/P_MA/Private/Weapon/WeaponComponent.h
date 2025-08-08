// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WeaponComponent.generated.h"

class USkeletalMeshComponent;
class USceneComponent;
class AActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UWeaponComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWeaponComponent();

	// Meshes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Mesh")
	TObjectPtr<USkeletalMeshComponent> HandleMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Mesh")
	TObjectPtr<USkeletalMeshComponent> BladeMesh = nullptr;
	
	// Marker
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Markers")
	TObjectPtr<USceneComponent> BaseMarker = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Markers")
	TObjectPtr<USceneComponent> TipMarker = nullptr;
	
	// Range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float Range = 100.f;
	
	// Editor
	UFUNCTION(CallInEditor, Category="Weapon")
	void MeasureRangeFromMarkers();
	
	// Getter
	UFUNCTION(BlueprintCallable, Category="Weapon")
	FVector GetBladeForwardVector() const;

	UFUNCTION(BlueprintCallable, Category="Weapon")
	FVector GetBladeBaseWorldLocation() const;

	//Helper ★ 액터 생성자에서만 호출
	static void CreateDefaultPartsForOwner(AActor* Owner, UWeaponComponent* Weapon, USceneComponent* Parent = nullptr);
};
