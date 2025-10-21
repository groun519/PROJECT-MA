// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlatformMatrixComponent.generated.h"

class UPlatformComponent;

UCLASS(Blueprintable)
class P_MA_API UPlatformMatrixComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPlatformMatrixComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
						   FActorComponentTickFunction* ThisTickFunction) override;

private:

	UPROPERTY(EditAnywhere, Category="Grid")
	TArray<UPlatformComponent*> Platforms;
	TArray<bool> PlatformsStatus;
	
	int32 Cols = 9;

	void SetPlatformEnable(int32 X, int32 Y);
	FORCEINLINE int32 GetIndex(int32 X, int32 Y) const { return Y * Cols + X; }
};
