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
// InteractableComponent->CALL_SETUP_HIGHLIGHTER(HighlightComponent);
// Set DefaultInteractKeyWidgetClass in MA Game Settings.
#define CALL_SETUP_INTERACT(MethodName) SetupInteraction(this, &std::remove_pointer_t<decltype(this)>::MethodName)
#define CALL_SETUP_HIGHLIGHTER(Highlighter) HighlightComponent = Highlighter

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

	void RequestInteract(AMAPlayerCharacter* Interactor);
	void SetInteractFocused(AMAPlayerCharacter* Interactor, bool bNewFocused);

	TWeakObjectPtr<UMAHighlightComponent> HighlightComponent;
	
private:
	void AttachKeyWidgetToInteractable();

	UPROPERTY(VisibleAnywhere, Category="MA|UI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWidgetComponent> InteractKeyWidgetComp;
	
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	TFunction<void(AMAPlayerCharacter*)> InteractionHandler;
	bool bFocused = false;
};

