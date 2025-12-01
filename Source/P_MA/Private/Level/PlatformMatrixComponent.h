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

	void SetPlatformEnable(int32 X, int32 Y);
	FORCEINLINE int32 GetIndex(int32 X, int32 Y) const { return Y * Cols + X; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grid")
	int32 Cols = 9;

	UPROPERTY(Transient)
	TArray<UPlatformComponent*> Platforms;
};
