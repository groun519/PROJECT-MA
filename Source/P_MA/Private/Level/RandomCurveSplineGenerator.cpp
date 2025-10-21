#include "RandomCurveSplineGenerator.h"
#include "Components/SplineComponent.h"

// Called when the game starts or when spawned
void ARandomCurveSplineGenerator::BeginPlay()
{
    Super::BeginPlay();

    // 동적으로 Spline 컴포넌트 생성
    USplineComponent* Spline = NewObject<USplineComponent>(this);  // NewObject를 사용하여 생성
    if (Spline)
    {
        // Spline을 등록하고, RootComponent로 설정
        Spline->RegisterComponent();
        RootComponent = Spline;

        // 시작점과 끝점을 기준으로 Spline을 생성
        Spline->AddSplinePoint(StartPoint, ESplineCoordinateSpace::Local);
        Spline->AddSplinePoint(EndPoint, ESplineCoordinateSpace::Local);

        // 랜덤 포인트를 추가하여 Spline의 곡선 생성
        for (int i = 1; i < 10; ++i)  // 더 많은 랜덤 포인트 추가
        {
            FVector RandomPoint = FVector(FMath::RandRange(-500, 500), FMath::RandRange(-500, 500), 0);
            Spline->AddSplinePoint(RandomPoint, ESplineCoordinateSpace::Local);
        }

        // 길이를 계산하고 일정한 길이가 되도록 마지막 포인트 보정
        float TotalLength = Spline->GetSplineLength();
        if (TotalLength > MaxLength)
        {
            // 길이를 MaxLength로 보정
            FVector AdjustedEnd = StartPoint + (EndPoint - StartPoint).GetSafeNormal() * MaxLength;
            int LastPointIndex = Spline->GetNumberOfSplinePoints() - 1;
            Spline->SetLocationAtSplinePoint(LastPointIndex, AdjustedEnd, ESplineCoordinateSpace::Local);
        }
    }
}
