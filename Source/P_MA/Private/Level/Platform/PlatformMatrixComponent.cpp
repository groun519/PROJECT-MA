// Fill out your copyright notice in the Description page of Project Settings.

#include "PlatformMatrixComponent.h"
#include "PlatformComponent.h"
#include "DrawDebugHelpers.h"
#include "P_MA/P_MA.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"

UPlatformMatrixComponent::UPlatformMatrixComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlatformMatrixComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlatformMatrixComponent::InitMatrix()
{
	Platforms.SetNum(GetCols() * GetCols());
	CreatePlatforms();
	SetMovedInPlatforms(true);
}

void UPlatformMatrixComponent::SetPlatformEnable(int32 X, int32 Y)
{
	int32 Index = GetIndex(X, Y);
	if (Platforms.IsValidIndex(Index))
	{
		Platforms[Index]->EnablePlatform();
	}
}

void UPlatformMatrixComponent::SetMovedInPlatforms(bool NewCanMovedIn)
{
	for (UPlatformComponent* Platform : Platforms)
	{
		Platform->SetCanMoveIn(NewCanMovedIn);
	}
}

void UPlatformMatrixComponent::CreatePlatforms()
{
	int32 OddCols = GetCols();
	
	for (int32 X = 0; X < OddCols; ++X)
	{
		for (int32 Y = 0; Y < OddCols; ++Y)
		{
			const int32 Index = GetIndex(X, Y);

			FString CompName = FString::Printf(TEXT("RuntimePlatform_%02d_%02d"), X, Y);

			UPlatformComponent* Platform = NewObject<UPlatformComponent>(this, *CompName);
			if (Platform)
			{
				Platform->CreationMethod = EComponentCreationMethod::Instance;
				Platform->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
				Platform->RegisterComponent();
				Platform->SetRelativeLocation(FVector(-(X - OddCols / 2) * 200.f, (Y - OddCols / 2) * 200.f, 0.f));
				Platform->InitReadyWall();
			
				if (PlatformMaterial)
				{
					Platform->SetMaterial(0, PlatformMaterial);
				}

				/** 센터 3*3 Enable **/
				int32 Center = OddCols / 2; 
				bool bIsCenter3x3 = (FMath::Abs(X - Center) <= 1) && (FMath::Abs(Y - Center) <= 1);
				if (bIsCenter3x3)
				{
					Platform->EnablePlatform(); 
				}
				else
				{
					Platform->SetVisibility(false, true);
					Platform->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}

				/** Platform 배열에 저장 **/
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
}

void UPlatformMatrixComponent::ResolveReadyWallOverlapsOnce()
{
	if (!GetWorld()) return;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_Hitbox);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ResolveReadyWallOverlapsOnce), false);
	QueryParams.AddIgnoredActor(GetOwner());

	for (UPlatformComponent* Platform : Platforms)
	{
		if (!Platform || !Platform->IsEnablePlatform() || !Platform->ReadyWallBox)
			continue;

		const FVector QueryCenter = Platform->ReadyWallBox->GetComponentLocation();
		const FQuat QueryRotation = Platform->ReadyWallBox->GetComponentQuat();
		const FVector QueryBoxExtent = Platform->ReadyWallBox->GetScaledBoxExtent();
		const FCollisionShape QueryShape = FCollisionShape::MakeBox(QueryBoxExtent);

		TArray<FOverlapResult> Overlaps;
		if (!GetWorld()->OverlapMultiByObjectType(Overlaps, QueryCenter, QueryRotation, ObjectQueryParams, QueryShape, QueryParams))
			continue;

		for (const FOverlapResult& Overlap : Overlaps)
		{
			AActor* OverlapActor = Overlap.GetActor();
			if (!OverlapActor) continue;

			Platform->OnWallOverlap(
				Platform->ReadyWallBox,
				OverlapActor,
				nullptr,
				INDEX_NONE,
				false,
				FHitResult());
		}
	}
}


