// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugShapeHelper.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"

void FDebugShapeHelper::DrawDebugSectorableCircle(
    UWorld* World,
    const FVector& Center,
    float Radius,
    int32 Segments,
    bool bUseSector,
    float HalfAngleDeg,
    FVector Forward,
    FColor Color,
    float Thickness)
{
    if (!World || Segments < 3) return;

    if (!bUseSector) // 원
    {
        const float AngleStep = 2 * PI / Segments;
        FVector PrevPoint = Center + Radius * FVector(FMath::Cos(0.f), FMath::Sin(0.f), 0);

        for (int32 i = 1; i <= Segments; i++)
        {
            float Angle = i * AngleStep;
            FVector NextPoint = Center + Radius * FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0);

            DrawDebugLine(World, PrevPoint, NextPoint, Color, false, 3, 0, Thickness);
            PrevPoint = NextPoint;
        }
    }
    else // 부채꼴
    {
        Forward = Forward.GetSafeNormal2D();
        FVector Right = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

        float StartRad = FMath::DegreesToRadians(-HalfAngleDeg);
        float EndRad   = FMath::DegreesToRadians(HalfAngleDeg);
        float AngleStep = (EndRad - StartRad) / Segments;

        FVector PrevPoint = Center + Radius * (FMath::Cos(StartRad) * Forward + FMath::Sin(StartRad) * Right);

        for (int32 i = 1; i <= Segments; i++)
        {
            float Angle = StartRad + i * AngleStep;
            FVector NextPoint = Center + Radius * (FMath::Cos(Angle) * Forward + FMath::Sin(Angle) * Right);

            DrawDebugLine(World, PrevPoint, NextPoint, Color, false, 3, 0, Thickness);
            PrevPoint = NextPoint;

            if (i == 1 || i == Segments)
            {
                DrawDebugLine(World, Center, NextPoint, Color, false, 3, 0, Thickness);
            }
        }
    }
}

void FDebugShapeHelper::DrawDebugRect(
    UWorld* World,
    const FVector& Center,
    float HalfX,
    float HalfY,
    FVector Forward,
    FColor Color,
    float Thickness)
{
    if (!World) return;

    Forward = Forward.GetSafeNormal2D();
    FVector Right = FVector::CrossProduct(FVector::UpVector, Forward).GetSafeNormal();

    FVector P1 = Center + Forward * HalfY + Right * HalfX;
    FVector P2 = Center + Forward * HalfY - Right * HalfX;
    FVector P3 = Center - Forward * HalfY - Right * HalfX;
    FVector P4 = Center - Forward * HalfY + Right * HalfX;

    DrawDebugLine(World, P1, P2, Color, false, 3, 0, Thickness);
    DrawDebugLine(World, P2, P3, Color, false, 3, 0, Thickness);
    DrawDebugLine(World, P3, P4, Color, false, 3, 0, Thickness);
    DrawDebugLine(World, P4, P1, Color, false, 3, 0, Thickness);
}

void FDebugShapeHelper::ConvertOverlapsToHitResults(
    const TArray<FOverlapResult>& Overlaps,
    TArray<FHitResult>& OutHits)
{
    OutHits.Reset();

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!HitActor) continue;

        // FHitResult 전용 생성자 사용
        FHitResult Converted(
            HitActor,
            Overlap.Component.Get(),
            HitActor->GetActorLocation(), // ImpactPoint 대체
            FVector::UpVector              // ImpactNormal 대체
        );

        OutHits.Add(Converted);
    }
}
