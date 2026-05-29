#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MAOverlayComponent.generated.h"

class UMaterialInstanceDynamic;
class UMaterialInterface;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom))
class P_MA_API UMAOverlayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAOverlayComponent();

	UMaterialInstanceDynamic* AddTimedOverlay(UMaterialInterface* Material, int32 Priority, float Duration);
	UMaterialInstanceDynamic* AddPersistentOverlay(UMaterialInterface* Material, int32 Priority);
	void RemovePersistentOverlay(UMaterialInterface* Material);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	struct FOverlayRequest
	{
		TObjectPtr<UMaterialInterface> Material;
		TObjectPtr<UMaterialInstanceDynamic> MID;
		int32 Priority = 0;
		float RemainingTime = -1.f;
	};

	UMaterialInstanceDynamic* AddOverlay(UMaterialInterface* Material, int32 Priority, float RemainingTime);
	void RefreshActiveOverlay();
	void RefreshTickEnabled();
	USkeletalMeshComponent* ResolveTargetMesh() const;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> TargetMesh;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ActiveMID;

	TArray<FOverlayRequest> OverlayRequests;
};
