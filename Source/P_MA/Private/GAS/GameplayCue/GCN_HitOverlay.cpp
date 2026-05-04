// Fill out your copyright notice in the Description page of Project Settings.

#include "GAS/GameplayCue/GCN_HitOverlay.h"

#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
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

	USkeletalMeshComponent* MeshComp = ResolveTargetMesh(MyTarget);
	if (!MeshComp) return;

	StartOrRestartFade(MeshComp, ResolveOverlayColor(Parameters));
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
		USkeletalMeshComponent* MeshComp = FadeState.MeshComp.Get();
		UMaterialInstanceDynamic* OverlayMID = FadeState.OverlayMID.Get();
		if (!MeshComp || !OverlayMID)
		{
			if (MeshComp)
			{
				MeshComp->SetOverlayMaterial(nullptr);
			}
			ActiveFades.RemoveAtSwap(Index);
			continue;
		}

		FadeState.Elapsed += DeltaSeconds;
		const float Alpha = FMath::Clamp(FadeState.Elapsed / SafeFadeDuration, 0.f, 1.f);
		OverlayMID->SetScalarParameterValue(OpacityParamName, 1.f - Alpha);

		if (Alpha >= 1.f)
		{
			MeshComp->SetOverlayMaterial(nullptr);
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
	if (Parameters.OriginalTag.IsValid())
	{
		return Parameters.OriginalTag;
	}

	const FGameplayTag OverlayRootTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Hit.Overlay"));
	for (const FGameplayTag& SourceTag : Parameters.AggregatedSourceTags)
	{
		if (SourceTag.MatchesTag(OverlayRootTag))
		{
			return SourceTag;
		}
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

void AGCN_HitOverlay::StartOrRestartFade(USkeletalMeshComponent* MeshComp, const FLinearColor& OverlayColor)
{
	if (!MeshComp || !OverlayMaterial) return;

	UMaterialInstanceDynamic* OverlayMID = UMaterialInstanceDynamic::Create(OverlayMaterial, this);
	if (!OverlayMID) return;

	OverlayMID->SetScalarParameterValue(OpacityParamName, 1.f);
	OverlayMID->SetVectorParameterValue(ColorParamName, OverlayColor);
	MeshComp->SetOverlayMaterial(OverlayMID);

	const int32 ExistingIndex = FindFadeIndex(MeshComp);
	if (ExistingIndex == INDEX_NONE)
	{
		FActiveOverlayFade NewFade;
		NewFade.MeshComp = MeshComp;
		NewFade.OverlayMID = OverlayMID;
		NewFade.Elapsed = 0.f;
		ActiveFades.Add(MoveTemp(NewFade));
	}
	else
	{
		ActiveFades[ExistingIndex].OverlayMID = OverlayMID;
		ActiveFades[ExistingIndex].Elapsed = 0.f;
	}

	SetActorTickEnabled(true);
}

int32 AGCN_HitOverlay::FindFadeIndex(const USkeletalMeshComponent* MeshComp) const
{
	for (int32 Index = 0; Index < ActiveFades.Num(); ++Index)
	{
		if (ActiveFades[Index].MeshComp.Get() == MeshComp)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

USkeletalMeshComponent* AGCN_HitOverlay::ResolveTargetMesh(AActor* TargetActor) const
{
	if (!TargetActor) return nullptr;

	if (ACharacter* Character = Cast<ACharacter>(TargetActor))
	{
		return Character->GetMesh();
	}

	return TargetActor->FindComponentByClass<USkeletalMeshComponent>();
}

