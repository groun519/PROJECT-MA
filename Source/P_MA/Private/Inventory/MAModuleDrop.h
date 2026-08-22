#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/MASkillModuleTypes.h"
#include "GameFramework/Actor.h"
#include "MAModuleDrop.generated.h"

class AMAPlayerCharacter;
class UMAHighlightComponent;
class UMAInteractableComponent;
class UMASkillModule;
class UMASkillTooltipWidget;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
class UWidgetComponent;
struct FMAModuleRarityData;

USTRUCT(BlueprintType)
struct FMAModuleDropData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drop", meta=(ClampMin="1"))
	int32 ModuleId = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Drop", meta=(ClampMin="1"))
	int32 Count = 1;

	UPROPERTY()
	FVector_NetQuantize VisualOrigin;

	UPROPERTY()
	bool bInFlight = false;

	bool IsValid() const { return ModuleId > 0 && Count > 0; }
};

/** Replicated world representation and pickup transaction for a built skill module. */
UCLASS()
class P_MA_API AMAModuleDrop : public AActor
{
	GENERATED_BODY()

public:
	AMAModuleDrop();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool InitializeDrop(int32 ModuleId, int32 Count = 1);
	void DropNear(const FVector& Origin, int32 DropCount);

protected:
	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAInteractableComponent> InteractableComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<USceneComponent> DropVisualRoot;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UStaticMeshComponent> DropMeshComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UMAHighlightComponent> HighlightComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UStaticMeshComponent> RarityVisualComponent;

	UPROPERTY(VisibleAnywhere, Category="Component")
	TObjectPtr<UWidgetComponent> TooltipWidgetComponent;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FMAStaticMeshVisualData ModuleMesh;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FMAStaticMeshVisualData SubModuleMesh;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FMAStaticMeshVisualData DefaultItemMesh;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	FMAStaticMeshVisualData FlightMesh;

	UPROPERTY(EditDefaultsOnly, Category="Visual")
	TSubclassOf<UMASkillTooltipWidget> TooltipWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="Drop|Placement", meta=(ClampMin="0.0"))
	float BaseDropDistance = 90.f;

	UPROPERTY(EditDefaultsOnly, Category="Drop|Placement", meta=(ClampMin="0.0"))
	float DropDistancePerItem = 35.f;

	UPROPERTY(EditDefaultsOnly, Category="Drop|Animation", meta=(ClampMin="0.0"))
	float DropAnimationDuration = 0.45f;

	UPROPERTY(EditDefaultsOnly, Category="Drop|Animation", meta=(ClampMin="0.0"))
	float DropArcHeight = 120.f;

	UPROPERTY(EditInstanceOnly, ReplicatedUsing=OnRep_DropData, Category="Drop")
	FMAModuleDropData DropData;

private:
	void HandleInteract(AMAPlayerCharacter* Interactor);
	void HandleCursorHover(bool bHovered);
	FVector FindDropLocationNear(const FVector& Origin, int32 DropCount) const;
	void StartDropAnimation();
	void FinishDropAnimation();
	void ApplyMeshVisual(const FMAStaticMeshVisualData* VisualData);
	void RefreshPresentation();
	void RefreshTooltip();
	const FMAStaticMeshVisualData* ResolveDropMesh() const;
	const FMAModuleRarityData* ResolveRarityData() const;

	UFUNCTION()
	void OnRep_DropData();

	UPROPERTY(Transient)
	TObjectPtr<UMASkillModule> CachedModule;

	float DropAnimationElapsed = 0.f;
	bool bDropAnimationActive = false;
	bool bPickupInProgress = false;
};
