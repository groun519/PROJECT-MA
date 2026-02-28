// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Actor.h"
#include "GCN_HitOverlay.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class USkeletalMeshComponent;

UCLASS()
class P_MA_API AGCN_HitOverlay : public AGameplayCueNotify_Actor
{
	GENERATED_BODY()

public:
	AGCN_HitOverlay();

	virtual void HandleGameplayCue(AActor* MyTarget, EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Overlay")
	TObjectPtr<UMaterialInterface> OverlayMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Overlay")
	FName OpacityParamName = TEXT("Opacity");

	UPROPERTY(EditDefaultsOnly, Category = "Overlay", meta = (ClampMin = "0.01"))
	float FadeDuration = 0.08f;

private:
	struct FActiveOverlayFade
	{
		TWeakObjectPtr<USkeletalMeshComponent> MeshComp;
		TWeakObjectPtr<UMaterialInstanceDynamic> OverlayMID;
		float Elapsed = 0.f;
	};

	void StartOrRestartFade(USkeletalMeshComponent* MeshComp);
	int32 FindFadeIndex(const USkeletalMeshComponent* MeshComp) const;
	USkeletalMeshComponent* ResolveTargetMesh(AActor* TargetActor) const;

	TArray<FActiveOverlayFade> ActiveFades;
};
