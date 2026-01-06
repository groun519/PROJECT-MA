// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlatformMatrixComponent.generated.h"

class UPlatformComponent;

UCLASS(Blueprintable)
class P_MA_API UPlatformMatrixComponent : public USceneComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UPlatformMatrixComponent();
	void InitMatrix();

	UPROPERTY(EditAnywhere, Category="Grid")
	int32 Cols = 9;

	UPROPERTY(EditAnywhere, Category="Platform")
	UMaterialInterface* PlatformMaterial;

	/** Platform **/
	UPROPERTY(Transient)
	TArray<UPlatformComponent*> Platforms;

	void SetPlatformEnable(int32 X, int32 Y);
	FORCEINLINE int32 GetIndex(int32 X, int32 Y) const { return Y * Cols + X; }

	/** Debug **/
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDebugPlatformNumAtFirstFrame = false;
	
private:
	void CreatePlatforms();
};
