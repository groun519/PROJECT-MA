#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MACameraOcclusionCutoutComponent.generated.h"

class APlayerController;
class UMaterialParameterCollection;

/**
 * Reveals one local view target through opt-in environment materials.
 *
 * The component owns the camera/target tracking and the material parameter contract.
 * Callers only select when the feature is active and which actor must remain visible.
 */
UCLASS(ClassGroup=(Camera), meta=(BlueprintSpawnableComponent))
class P_MA_API UMACameraOcclusionCutoutComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMACameraOcclusionCutoutComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void RevealTarget(APlayerController& Viewer, AActor& Target);
	void ClearTarget();

private:
	void UpdateMaterialParameters();
	void SetCutoutEnabled(bool bEnabled) const;

	UPROPERTY(EditAnywhere, Category = "Camera|Occlusion Cutout")
	TObjectPtr<UMaterialParameterCollection> ParameterCollection;

	/** Radius at the target plane. The material scales it toward the camera to preserve its screen size. */
	UPROPERTY(EditAnywhere, Category = "Camera|Occlusion Cutout", meta = (ClampMin = "1.0"))
	float CutoutRadius = 125.f;

	TWeakObjectPtr<APlayerController> ViewerController;
	TWeakObjectPtr<AActor> RevealTargetActor;
};
