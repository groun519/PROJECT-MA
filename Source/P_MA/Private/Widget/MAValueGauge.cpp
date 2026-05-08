// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/MAValueGauge.h"
#include "Components/ProgressBar.h"
#include "AbilitySystemComponent.h"
#include "Components/TextBlock.h"
#include "TimerManager.h" // 💡 타이머를 위해 추가
#include "Math/UnrealMathUtility.h" // 💡 FInterpTo (서서히 줄어드는 수학 함수)를 위해 추가

void UMAValueGauge::NativePreConstruct()
{
    Super::NativePreConstruct();
    HealthBar->SetFillColorAndOpacity(BarColor);

    ValueText->SetFont(ValueTextFont);

    ValueText->SetVisibility(bValueTextVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    HealthBar->SetVisibility(bProgressBarVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

    // 고스트 바 가시성 설정
    if (GhostProgressBar)
    {
        GhostProgressBar->SetVisibility(bProgressBarVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        // 고스트 바는 항상 하얀색으로 고정
        GhostProgressBar->SetFillColorAndOpacity(FLinearColor::White); 
    }
}

void UMAValueGauge::SetAndBoundToGameplayAttribute(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayAttribute& Attribute, const FGameplayAttribute& MaxAttribute)
{
    // 범님 원본 100% 동일
    if (AbilitySystemComponent)
    {
       bool bFound;
       float Value = AbilitySystemComponent->GetGameplayAttributeValue(Attribute, bFound);
       float MaxValue = AbilitySystemComponent->GetGameplayAttributeValue(MaxAttribute, bFound);
       if (bFound)
       {
          SetValue(Value, MaxValue);
       }

       AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &UMAValueGauge::ValueChanged);
       AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(MaxAttribute).AddUObject(this, &UMAValueGauge::MaxValueChanged);
    }
}

void UMAValueGauge::SetValue(float NewValue, float NewMaxValue)
{
    CachedValue = NewValue;
    CachedMaxValue = NewMaxValue;

    if (NewMaxValue == 0) return;

    // 1. 목표 퍼센트 계산 및 메인 바 즉시 깎기
    TargetPercent = NewValue / NewMaxValue;
    HealthBar->SetPercent(TargetPercent);

    // ✨ 2. 고스트 바 타이머 로직
    if (CurrentGhostPercent <= TargetPercent) 
    {
        // 피가 차오를 때: 하얀 바도 즉시 채우고 타이머 끄기
        CurrentGhostPercent = TargetPercent;
        if (GhostProgressBar) GhostProgressBar->SetPercent(CurrentGhostPercent);
        
        GetWorld()->GetTimerManager().ClearTimer(GhostTimerHandle);
    }
    else 
    {
        // 피가 깎였을 때: 하얀 바가 서서히 줄어들도록 타이머 켜기
        if (!GetWorld()->GetTimerManager().IsTimerActive(GhostTimerHandle))
        {
            GetWorld()->GetTimerManager().SetTimer(GhostTimerHandle, this, &UMAValueGauge::UpdateGhostBar, 0.016f, true);
        }
    }

    // 3. 텍스트 설정 (범님 원본 100% 동일)
    FNumberFormattingOptions FormatOps = FNumberFormattingOptions().SetMaximumFractionalDigits(0);
    ValueText->SetText(
       FText::Format(
          FTextFormat::FromString("{0}/{1}"),
          FText::AsNumber(NewValue, &FormatOps),
          FText::AsNumber(NewMaxValue, &FormatOps)
       )
    );
}

// ✨ 3. 타이머가 0.016초마다 호출해서 하얀 바를 서서히 줄이는 함수
void UMAValueGauge::UpdateGhostBar()
{
    CurrentGhostPercent = FMath::FInterpTo(CurrentGhostPercent, TargetPercent, 0.016f, 1.0f);

    if (GhostProgressBar)
    {
        GhostProgressBar->SetPercent(CurrentGhostPercent);
    }

    // 다 줄어들면 타이머 끄기
    if (FMath::IsNearlyEqual(CurrentGhostPercent, TargetPercent, 0.001f))
    {
        CurrentGhostPercent = TargetPercent;
        if (GhostProgressBar) GhostProgressBar->SetPercent(CurrentGhostPercent);
        
        GetWorld()->GetTimerManager().ClearTimer(GhostTimerHandle);
    }
}

void UMAValueGauge::ValueChanged(const FOnAttributeChangeData& ChangedData)
{
    SetValue(ChangedData.NewValue, CachedMaxValue);
}

void UMAValueGauge::MaxValueChanged(const FOnAttributeChangeData& ChangedData)
{
    SetValue(CachedValue, ChangedData.NewValue);
}