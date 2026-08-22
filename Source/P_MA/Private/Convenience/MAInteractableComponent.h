#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "MAInteractableComponent.generated.h"

class AMAPlayerCharacter;
class UMAHighlightComponent;
class UPrimitiveComponent;
class UWidgetComponent;

// Usage from an owning actor constructor:
// InteractableComponent->CALL_SETUP_INTERACT(HandleInteract);
// InteractableComponent->CALL_SETUP_FOCUS(HandleFocus);
// InteractableComponent->CALL_SETUP_CURSOR_HOVER(HandleCursorHover);
// InteractableComponent->CALL_SETUP_HIGHLIGHTER(HighlightComponent);
// Set DefaultInteractKeyWidgetClass in MA Game Settings.
#define CALL_SETUP_INTERACT(MethodName) SetupInteraction(this, &std::remove_pointer_t<decltype(this)>::MethodName)
#define CALL_SETUP_FOCUS(MethodName) SetupFocus(this, &std::remove_pointer_t<decltype(this)>::MethodName)
#define CALL_SETUP_CURSOR_HOVER(MethodName) SetupCursorHover(this, &std::remove_pointer_t<decltype(this)>::MethodName)
#define CALL_SETUP_HIGHLIGHTER(Highlighter) HighlightComponent = Highlighter
#define CALL_SETUP_INTERACTION_MODE(ModeName) ExecutionMode = EMAInteractionExecutionMode::ModeName

UENUM(BlueprintType)
enum class EMAInteractionExecutionMode : uint8
{
	Local,
	Server,
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UMAInteractableComponent : public USphereComponent
{
	GENERATED_BODY()

protected:
	virtual void OnRegister() override;
	virtual void BeginPlay() override;

public:
	UMAInteractableComponent();

	template<typename T>
	void SetupInteraction(T* InObj, void (T::*InMethod)(AMAPlayerCharacter*))
	{
		InteractionHandler = [InObj, InMethod](AMAPlayerCharacter* Interactor)
		{
			if (InObj && InMethod)
			{
				(InObj->*InMethod)(Interactor);
			}
		};
	}

	template<typename T>
	void SetupFocus(T* InObj, void (T::*InMethod)(AMAPlayerCharacter*, bool))
	{
		FocusHandler = [InObj, InMethod](AMAPlayerCharacter* Interactor, const bool bFocused)
		{
			if (InObj && InMethod)
			{
				(InObj->*InMethod)(Interactor, bFocused);
			}
		};
	}

	template<typename T>
	void SetupCursorHover(T* InObj, void (T::*InMethod)(bool))
	{
		CursorHoverHandler = [InObj, InMethod](const bool bHovered)
		{
			if (InObj && InMethod)
			{
				(InObj->*InMethod)(bHovered);
			}
		};
	}

	void AddCursorHoverTarget(UPrimitiveComponent* Target);
	bool IsCursorHoverTarget(const UPrimitiveComponent* Target) const;
	void RequestInteract(AMAPlayerCharacter* Interactor);
	void SetInteractFocused(AMAPlayerCharacter* Interactor, bool bNewFocused);
	void SetCursorHovered(bool bNewHovered);
	bool CanServerInteract(const AMAPlayerCharacter* Interactor) const;

	TWeakObjectPtr<UMAHighlightComponent> HighlightComponent;

	UPROPERTY(EditAnywhere, Category="MA|Interaction")
	EMAInteractionExecutionMode ExecutionMode = EMAInteractionExecutionMode::Local;
	
private:
	void AttachKeyWidgetToInteractable();

	UPROPERTY(VisibleAnywhere, Category="MA|UI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWidgetComponent> InteractKeyWidgetComp;
	
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	TFunction<void(AMAPlayerCharacter*)> InteractionHandler;
	TFunction<void(AMAPlayerCharacter*, bool)> FocusHandler;
	TFunction<void(bool)> CursorHoverHandler;
	TArray<TWeakObjectPtr<UPrimitiveComponent>> CursorHoverTargets;
	bool bFocused = false;
	bool bCursorHovered = false;

};

