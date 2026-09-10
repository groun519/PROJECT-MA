#pragma once

#include "CoreMinimal.h"
#include "Tickable.h"
#include "UObject/Object.h"
#include "MASpaceTransitionMask.generated.h"

class APostProcessVolume;
class UMaterialInstanceDynamic;
class UMASpaceTransitionVisibilityComponent;

/** Presents one local Space transition mask from Close through Open. */
UCLASS()
class P_MA_API UMASpaceTransitionMask : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UMASpaceTransitionMask(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UWorld* GetWorld() const override;
	virtual void BeginDestroy() override;

	bool Close(const FVector& Center, FSimpleDelegate OnClosed = FSimpleDelegate());
	bool Open(const FVector& Center, FSimpleDelegate OnOpened = FSimpleDelegate());
	void Reset();

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }

private:
	enum class EPhase : uint8
	{
		Open,
		Closing,
		Closed,
		Opening
	};

	bool CreateMask();
	void UpdateRadius() const;
	void CollectVisibleSubjects();
	void ReleaseMask();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> TransitionMaterialInstance;

	TWeakObjectPtr<APostProcessVolume> TransitionVolume;
	TArray<TWeakObjectPtr<UMASpaceTransitionVisibilityComponent>> ActiveVisibleSubjects;
	FSimpleDelegate TransitionFinishedDelegate;
	EPhase Phase = EPhase::Open;
	float TransitionAlpha = 1.f;

	static constexpr float OpenRadius = 3250.f;
	static constexpr float TransitionDuration = 2.25f;
};
