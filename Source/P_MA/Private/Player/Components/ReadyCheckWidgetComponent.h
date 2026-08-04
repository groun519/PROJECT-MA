#pragma once

#include "CoreMinimal.h"
#include "Framework/MAGameStateTypes.h"
#include "Components/WidgetComponent.h"
#include "ReadyCheckWidgetComponent.generated.h"

class UReadyStateComponent;

UCLASS(ClassGroup=(UI), meta=(BlueprintSpawnableComponent))
class P_MA_API UReadyCheckWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UReadyCheckWidgetComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BindReadyStateDelegates();
	void BindSectorStateDelegate();
	void HandleReadyStateChanged(bool bIsReady);
	void HandleSectorStateChanged(EMASectorState NewState);

	void RequestReadyCheckPresentation();
	void FlushReadyCheckPresentation();
	void ApplyReadyCheckPresentation(EMASectorState InState);
	EMASectorState ResolveReadyCheckSectorState() const;

	UPROPERTY(Transient)
	TObjectPtr<UReadyStateComponent> CachedReadyStateComponent;

	TWeakObjectPtr<class AMAGameState> CachedGameState;
	bool bReadyCheckPresentationPending = false;
};
