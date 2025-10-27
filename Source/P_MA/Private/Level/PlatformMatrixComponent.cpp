// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformMatrixComponent.h"
#include "PlatformComponent.h"


UPlatformMatrixComponent::UPlatformMatrixComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	/** Set Matrix **/
	Platforms.SetNum(Cols * Cols);
	PlatformsStatus.SetNum(Cols * Cols);

	for (int32 Y = 0; Y < Cols; ++Y)
	{
		for (int32 X = 0; X < Cols; ++X)
		{
			const int32 Index = Y * Cols + X;
			FString CompName = FString::Printf(TEXT("Platform_%02d_%02d"), Y, X);

			UPlatformComponent* Platform = CreateDefaultSubobject<UPlatformComponent>(*CompName);
			Platform->SetupAttachment(this);
			Platform->SetRelativeLocation(FVector((Y-Cols/2) * 200.f, (X-Cols/2) * 200.f, 0.f));
			Platform->SetRelativeScale3D(FVector(2.f, 2.f, 0.5f));

			Platforms[Index] = Platform;
			PlatformsStatus[Index] = true;
		}
	}
}

void UPlatformMatrixComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UPlatformMatrixComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlatformMatrixComponent::SetPlatformEnable(int32 X, int32 Y)
{
	Platforms[GetIndex(X, Y)]->EnablePlatform();
}

