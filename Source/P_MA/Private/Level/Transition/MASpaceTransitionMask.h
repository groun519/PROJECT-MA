#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MASpaceTransitionMask.generated.h"

class APostProcessVolume;
class UMaterialInstanceDynamic;
class UMASpaceTransitionVisibilityComponent;

/** Presents one local Space transition mask from Close through Open. */
UCLASS()
class P_MA_API UMASpaceTransitionMask : public UObject
{
	GENERATED_BODY()

public:
	virtual UWorld* GetWorld() const override;

	bool Close(const FVector& Center);
	bool Open(const FVector& Center);
	void SetProgress(float Progress) const;
	void Reset();

private:
	bool CreateMask();
	void SetCenter(const FVector& Center) const;
	void CollectVisibleSubjects();
	void ReleaseMask();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> TransitionMaterialInstance;

	TWeakObjectPtr<APostProcessVolume> TransitionVolume;
	TArray<TWeakObjectPtr<UMASpaceTransitionVisibilityComponent>> ActiveVisibleSubjects;

	static constexpr float OpenRadius = 3250.f;
};
