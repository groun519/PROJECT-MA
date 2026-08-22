#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MAReviveActor.generated.h"

class AMAPlayerCharacter;
class UDecalComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;
class USphereComponent;

UCLASS()
class P_MA_API AMAReviveActor : public AActor
{
	GENERATED_BODY()

public:
	AMAReviveActor();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void InitializeReviveTarget(AMAPlayerCharacter* InTargetPlayer);

private:
	UPROPERTY(VisibleDefaultsOnly, Category="Component")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleDefaultsOnly, Category="Component")
	TObjectPtr<USphereComponent> ReviveArea;

	UPROPERTY(VisibleDefaultsOnly, Category="Component")
	TObjectPtr<UDecalComponent> ReviveDecal;

	UPROPERTY(VisibleDefaultsOnly, Category="Component")
	TObjectPtr<UDecalComponent> ReviveBackgroundDecal;

	UPROPERTY(EditDefaultsOnly, Category="Revive", meta=(ClampMin="0.0", UIMin="0.0"))
	float ReviveRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category="Revive", meta=(ClampMin="0.0", UIMin="0.0"))
	float ReviveProgressPerSecondPerPlayer = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category="Revive|Automatic", meta=(ClampMin="0.0", ClampMax="0.99", UIMin="0.0", UIMax="0.99"))
	float AutomaticReviveProgressLimit = 0.9f;

	UPROPERTY(EditDefaultsOnly, Category="Revive|Automatic", meta=(ClampMin="0.01", UIMin="0.01"))
	float AutomaticReviveDuration = 30.f;

	UPROPERTY(EditDefaultsOnly, Category="Revive|Visual", meta=(ClampMin="1.0", UIMin="1.0"))
	float DecalProjectionDepth = 100.f;

	UPROPERTY(EditDefaultsOnly, Category="Revive|Visual")
	TObjectPtr<UMaterialInterface> ReviveDecalMaterial;

	UPROPERTY(EditDefaultsOnly, Category="Revive|Visual")
	FLinearColor ReviveDecalColor = FLinearColor(1.f, 0.9f, 0.35f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category="Revive|Visual")
	FLinearColor ReviveBackgroundDecalColor = FLinearColor(0.35f, 0.35f, 0.3f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category="Revive|Visual")
	float DecalOpacity = 1.0f;

	UPROPERTY(Transient)
	TWeakObjectPtr<AMAPlayerCharacter> TargetPlayer;

	UPROPERTY(VisibleInstanceOnly, ReplicatedUsing=OnRep_ReviveProgress, Category="Revive")
	float CurrentReviveProgress = 0.f;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ReviveDecalMID;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ReviveBackgroundDecalMID;

	UFUNCTION()
	void OnRep_ReviveProgress();

	int32 CountRevivers() const;
	void CompleteRevive();
	void RefreshReviveDecalVisual();
	void RefreshReviveDecalProgress();
};
