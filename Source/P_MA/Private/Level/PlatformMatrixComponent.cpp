// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformMatrixComponent.h"
#include "PlatformComponent.h"

#include "DrawDebugHelpers.h"


UPlatformMatrixComponent::UPlatformMatrixComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// /** Set Matrix **/
	// Platforms.SetNum(Cols * Cols);
	//
	// for (int32 X = 0; X < Cols; ++X)
	// {
	// 	for (int32 Y = 0; Y < Cols; ++Y)
	// 	{
	// 		const int32 Index = Y * Cols + X;
	// 		FString CompName = FString::Printf(TEXT("Platform_%02d_%02d"), X, Y);
	//
	// 		UPlatformComponent* Platform = CreateDefaultSubobject<UPlatformComponent>(*CompName);
	// 		Platform->SetupAttachment(this);
	// 		Platform->SetRelativeLocation(FVector(-(X-Cols/2) * 200.f, (Y-Cols/2) * 200.f, 0.f));
	// 		Platform->SetRelativeScale3D(FVector(2.f, 2.f, 0.5f));
	//
	// 		if (X < 3 || X > 5 || Y < 3 || Y > 5)
	// 		{
	// 			Platform->SetVisibility(false, true);
	// 			Platform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 		}
	// 		Platforms[Index] = Platform;
	// 	}
	// }
}

void UPlatformMatrixComponent::BeginPlay()
{
	Super::BeginPlay();

	Platforms.SetNum(Cols * Cols);

	// 플랫폼 생성
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

			if (X < 3 || X > 5 || Y < 3 || Y > 5)
			{
				Platform->SetVisibility(false, true);
				Platform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}

			Platforms[Index] = Platform;

#if WITH_EDITOR
			// const FVector TextLoc = Platform->GetComponentLocation();
			// const FString Label = FString::Printf(TEXT("%d,%d"), X, Y);
			//
			// DrawDebugString(
			// 	GetWorld(),
			// 	TextLoc,
			// 	Label,
			// 	nullptr,
			// 	FColor::Cyan,
			// 	-1.f,      // -1이면 무한 지속 (디버그용)
			// 	true,      // 그림자
			// 	1.f       // 폰트 스케일
			// );
#endif
		}
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