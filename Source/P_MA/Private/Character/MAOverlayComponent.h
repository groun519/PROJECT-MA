#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MAOverlayComponent.generated.h"

class UMaterialInstanceDynamic;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom))
class P_MA_API UMAOverlayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAOverlayComponent();

	UMaterialInstanceDynamic* GetOrCreateOverlay();

protected:
	virtual void BeginPlay() override;

private:
	USkeletalMeshComponent* ResolveTargetMesh() const;

	UPROPERTY(Transient)
	TObjectPtr<USkeletalMeshComponent> TargetMesh;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> OverlayMID;
};
