// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


UENUM(BlueprintType)
enum class EVA_Shape : uint8
{
	None,
	Line,
	Sphere,
	Box,
};

class FDebugShapeHelper
{
public:
    static void DrawDebugSectorableCircle(
        UWorld* World,
        const FVector& Center,
        float Radius,
        int32 Segments,
        bool bUseSector = false,
        float HalfAngleDeg = 0.f,
        FVector Forward = FVector::ForwardVector,
        FColor Color = FColor::Green,
        float Thickness = 1.f
    );

    static void DrawDebugRect(
        UWorld* World,
        const FVector& Center,
        float HalfX,
        float HalfY,
        FVector Forward = FVector::ForwardVector,
        FColor Color = FColor::Blue,
        float Thickness = 1.f
    );

	static void ConvertOverlapsToHitResults(
	const TArray<FOverlapResult>& Overlaps,
	TArray<FHitResult>& OutHits);
};