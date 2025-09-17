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
    const FVector Forward,
    FColor Color,
    float Thickness)
{
    if (!World || Segments < 3) return;

    // Forward → 정규화
    FVector Fwd = Forward.GetSafeNormal();
    // Right → Forward와 Up으로부터 직교축 생성
    FVector Right = FVector::CrossProduct(FVector::UpVector, Fwd).GetSafeNormal();

    float StartRad = bUseSector ? FMath::DegreesToRadians(-HalfAngleDeg) : 0.f;
    float EndRad   = bUseSector ? FMath::DegreesToRadians(HalfAngleDeg)  : 2 * PI;
    float AngleStep = (EndRad - StartRad) / Segments;

    FVector PrevPoint = Center + Radius * (FMath::Cos(StartRad) * Fwd + FMath::Sin(StartRad) * Right);

    for (int32 i = 1; i <= Segments; i++)
    {
        float Angle = StartRad + i * AngleStep;
        FVector NextPoint = Center + Radius * (FMath::Cos(Angle) * Fwd + FMath::Sin(Angle) * Right);

        DrawDebugLine(World, PrevPoint, NextPoint, Color, false, 3, 0, Thickness);
        PrevPoint = NextPoint;

        if (bUseSector && (i == 1 || i == Segments))
        {
            DrawDebugLine(World, Center, NextPoint, Color, false, 3, 0, Thickness);
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

    const FVector P1 = Center + Forward * HalfX + Right * HalfY;
    const FVector P2 = Center + Forward * HalfX - Right * HalfY;
    const FVector P3 = Center - Forward * HalfX - Right * HalfY;
    const FVector P4 = Center - Forward * HalfX + Right * HalfY;

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
