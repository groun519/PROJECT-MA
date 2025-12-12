// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformMatrixComponent.h"
#include "PlatformComponent.h"
#include "DrawDebugHelpers.h"

UPlatformMatrixComponent::UPlatformMatrixComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlatformMatrixComponent::BeginPlay()
{
	Super::BeginPlay();

	Platforms.SetNum(Cols * Cols); 
	CreatePlatforms();
}

void UPlatformMatrixComponent::SetPlatformEnable(int32 X, int32 Y)
{
	int32 Index = GetIndex(X, Y);
	if (Platforms.IsValidIndex(Index))
	{
		Platforms[Index]->EnablePlatform();
	}
}

void UPlatformMatrixComponent::CreatePlatforms()
{
	for (int32 X = 0; X < Cols; ++X)
	{
		for (int32 Y = 0; Y < Cols; ++Y)
		{
			const int32 Index = GetIndex(X, Y);

			FString CompName = FString::Printf(TEXT("RuntimePlatform_%02d_%02d"), X, Y);

			UPlatformComponent* Platform = NewObject<UPlatformComponent>(this, *CompName);

			if (!Platform) continue;

			Platform->RegisterComponent();

			Platform->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

			Platform->SetRelativeLocation(FVector(-(X - Cols / 2) * 200.f, (Y - Cols / 2) * 200.f, 0.f));
			Platform->SetRelativeScale3D(FVector(2.f, 2.f, 0.5f));

			if (PlatformMaterial)
			{
				Platform->SetMaterial(0, PlatformMaterial);
			}
			
			if (X < 3 || X > 5 || Y < 3 || Y > 5)
			{
				Platform->SetVisibility(false, true);
				Platform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			Platforms[Index] = Platform;

#if WITH_EDITOR
			if (bDebugPlatformNumAtFirstFrame)
			{
				const FVector TextLoc = Platform->GetComponentLocation();
				const FString Label = FString::Printf(TEXT("%d,%d"), X, Y);
				const float DebugTime = -1.f; // -1 -> 무한 지속
				const bool bOnTextShadow = true;
				const float FontSize = 1.f;
				
				DrawDebugString(
					GetWorld(),
					TextLoc,
					Label,
					nullptr,
					FColor::Cyan,
					DebugTime,
					bOnTextShadow,
					FontSize
				);
			}
#endif
		}
	}
}
