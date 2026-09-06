#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "MAPlayerSpectateComponent.generated.h"

class AMAPlayerCharacter;
class UEnhancedInputComponent;
class UInputAction;
class UInputMappingContext;
class UMASpectateOverlayWidget;

UCLASS(ClassGroup=(Player), meta=(BlueprintSpawnableComponent))
class P_MA_API UMAPlayerSpectateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMAPlayerSpectateComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void BindToPawn(AMAPlayerCharacter* PlayerCharacter);
	void BindInput(UEnhancedInputComponent* EnhancedInputComponent);
	void StopSpectating();

private:
	void RequestPawnCamera(float BlendTime = -1.f);
	void SpectateRight();
	void SpectateLeft();
	void HandleDeadTagChanged(const FGameplayTag Tag, int32 NewCount);
	void HandleSpectateTargetDeadTagChanged(const FGameplayTag Tag, int32 NewCount);
	UFUNCTION()
	void HandleSpectateTargetDestroyed(AActor* DestroyedActor);
	void SetSpectating(bool bNewSpectating);
	void RefreshSpectateTargets(AActor* DeadActor = nullptr);
	void SelectNextSpectateTarget(AActor* LostTarget);
	void ApplySpectateIndex(int32 NewIndex);
	void ApplyCameraTarget(AActor& Target, float BlendTime);
	AMAPlayerCharacter* GetCurrentSpectateTarget() const;
	FORCEINLINE APlayerController* GetLocalOwnerPlayerController() const
	{
		APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
		return PlayerController && PlayerController->IsLocalController() ? PlayerController : nullptr;
	}
	void SetSpectateInputMappingEnabled(bool bEnabled);
	void ClearBoundPawnDeathBinding();
	void BindSpectateTarget(AMAPlayerCharacter* SpectateTarget);
	void ClearSpectateTargetBinding();
	void ShowSpectateOverlay();
	void RemoveSpectateOverlay();

	UPROPERTY(EditDefaultsOnly, Category = "Spectate", meta = (ClampMin = "0.0"))
	float DeathSpectateBlendTime = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Spectate|Input")
	TObjectPtr<UInputAction> SpectateLeftInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Spectate|Input")
	TObjectPtr<UInputAction> SpectateRightInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Spectate|Input")
	TObjectPtr<UInputMappingContext> SpectateInputMapping;

	UPROPERTY(EditDefaultsOnly, Category = "Spectate|Input")
	int32 SpectateInputMappingPriority = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Spectate|UI")
	TSubclassOf<UMASpectateOverlayWidget> SpectateOverlayWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UMASpectateOverlayWidget> SpectateOverlayWidget;

	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<AMAPlayerCharacter>> SpectateTargets;

	TWeakObjectPtr<AMAPlayerCharacter> BoundPlayerCharacter;
	TWeakObjectPtr<AMAPlayerCharacter> ObservedSpectateTarget;
	FDelegateHandle DeadTagChangedHandle;
	FDelegateHandle SpectateTargetDeadTagChangedHandle;
	int32 CurrentSpectateIndex = INDEX_NONE;
	bool bSpectating = false;
};
