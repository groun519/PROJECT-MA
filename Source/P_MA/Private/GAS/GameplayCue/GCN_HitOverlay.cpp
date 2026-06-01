// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/GameplayCue/GCN_HitOverlay.h"

#include "Character/MAOverlayComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

AGCN_HitOverlay::AGCN_HitOverlay()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void AGCN_HitOverlay::HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	Super::HandleGameplayCue(MyTarget, EventType, Parameters);

	if (EventType != EGameplayCueEvent::Executed || !OverlayMaterial) return;

	StartFade(MyTarget, ResolveOverlayColor(Parameters));
}

void AGCN_HitOverlay::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ActiveFades.IsEmpty())
	{
		SetActorTickEnabled(false); return;
	}

	const float SafeFadeDuration = FMath::Max(FadeDuration, KINDA_SMALL_NUMBER);

	for (int32 Index = ActiveFades.Num() - 1; Index >= 0; --Index)
	{
		FActiveOverlayFade& FadeState = ActiveFades[Index];
		UMaterialInstanceDynamic* OverlayMID = FadeState.OverlayMID.Get();
		if (!OverlayMID)
		{
			ActiveFades.RemoveAtSwap(Index);
			continue;
		}

		FadeState.Elapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(FadeState.Elapsed / SafeFadeDuration, 0.f, 1.f);
		OverlayMID->SetScalarParameterValue(AlphaParamName, 1.f - Alpha);

		if (Alpha >= 1.f)
		{
			ActiveFades.RemoveAtSwap(Index);
		}
	}

	if (ActiveFades.IsEmpty())
	{
		SetActorTickEnabled(false);
	}
}

FGameplayTag AGCN_HitOverlay::ResolveRequestedCueTag(const FGameplayCueParameters& Parameters) const
{
	const FGameplayTag OverlayRootTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Hit.Overlay"));
	const auto IsOverlayColorTag = [&OverlayRootTag](const FGameplayTag& CueTag)
	{
		return CueTag.IsValid()
			&& CueTag.MatchesTag(OverlayRootTag)
			&& CueTag != OverlayRootTag;
	};

	if (IsOverlayColorTag(Parameters.OriginalTag))
	{
		return Parameters.OriginalTag;
	}

	for (const FGameplayTag& SourceTag : Parameters.AggregatedSourceTags)
	{
		if (IsOverlayColorTag(SourceTag))
		{
			return SourceTag;
		}
	}

	for (const FGameplayTag& TargetTag : Parameters.AggregatedTargetTags)
	{
		if (IsOverlayColorTag(TargetTag))
		{
			return TargetTag;
		}
	}

	if (IsOverlayColorTag(Parameters.MatchedTagName))
	{
		return Parameters.MatchedTagName;
	}

	return Parameters.MatchedTagName;
}

FLinearColor AGCN_HitOverlay::ResolveOverlayColor(const FGameplayCueParameters& Parameters) const
{
	const FGameplayTag RequestedCueTag = ResolveRequestedCueTag(Parameters);
	if (!RequestedCueTag.IsValid())
	{
		return DefaultOverlayColor;
	}

	FString ColorName;
	if (!RequestedCueTag.GetTagName().ToString().Split(TEXT("."), nullptr, &ColorName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		return DefaultOverlayColor;
	}

	if (ColorName.Equals(TEXT("Red"), ESearchCase::IgnoreCase)) return FLinearColor::Red;
	if (ColorName.Equals(TEXT("White"), ESearchCase::IgnoreCase)) return FLinearColor::White;
	if (ColorName.Equals(TEXT("Blue"), ESearchCase::IgnoreCase)) return FLinearColor::Blue;
	if (ColorName.Equals(TEXT("Green"), ESearchCase::IgnoreCase)) return FLinearColor::Green;
	if (ColorName.Equals(TEXT("Cyan"), ESearchCase::IgnoreCase)) return FLinearColor(0.f, 1.f, 1.f);

	return DefaultOverlayColor;
}

void AGCN_HitOverlay::StartFade(AActor* TargetActor, const FLinearColor& OverlayColor)
{
	if (!TargetActor || !OverlayMaterial) return;

	UMAOverlayComponent* OverlayComponent = TargetActor->FindComponentByClass<UMAOverlayComponent>();
	if (!OverlayComponent) return;

	UMaterialInstanceDynamic* OverlayMID = OverlayComponent->AddTimedOverlay(OverlayMaterial, 1, FadeDuration);
	if (!OverlayMID) return;

	OverlayMID->SetScalarParameterValue(AlphaParamName, 1.f);
	OverlayMID->SetVectorParameterValue(ColorParamName, OverlayColor);

	FActiveOverlayFade& FadeState = ActiveFades.AddDefaulted_GetRef();
	FadeState.OverlayMID = OverlayMID;
	FadeState.Elapsed = 0.f;
	SetActorTickEnabled(true);
}

