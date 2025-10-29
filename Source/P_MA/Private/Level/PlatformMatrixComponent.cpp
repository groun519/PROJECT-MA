// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformMatrixComponent.h"
#include "PlatformComponent.h"


UPlatformMatrixComponent::UPlatformMatrixComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	/** Set Matrix **/
	Platforms.SetNum(Cols * Cols);
	
	for (int32 X = 0; X < Cols; ++X)
	{
		for (int32 Y = 0; Y < Cols; ++Y)
		{
			const int32 Index = Y * Cols + X;
			FString CompName = FString::Printf(TEXT("Platform_%02d_%02d"), X, Y);
	
			UPlatformComponent* Platform = CreateDefaultSubobject<UPlatformComponent>(*CompName);
			Platform->SetupAttachment(this);
			Platform->SetRelativeLocation(FVector(-(X-Cols/2) * 200.f, (Y-Cols/2) * 200.f, 0.f));
			Platform->SetRelativeScale3D(FVector(2.f, 2.f, 0.5f));
	
			if (X < 3 || X > 5 || Y < 3 || Y > 5)
			{
				Platform->SetVisibility(false, true);
				Platform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
			Platforms[Index] = Platform;
		}
	}
}

void UPlatformMatrixComponent::OnRegister()
{
	Super::OnRegister();

	// 등록된 이후 트랜스폼 계층 확정
	for (UPlatformComponent* Platform : Platforms)
	{
		if (!Platform) continue;

		// SceneGraph에 실제로 Attach 갱신
		Platform->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	}
}

void UPlatformMatrixComponent::SetPlatformEnable(int32 X, int32 Y)
{
	int32 Index = GetIndex(X, Y);
	if (Platforms.IsValidIndex(Index))
	{
		Platforms[Index]->EnablePlatform();
	}
}