#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "InteractComponent.generated.h"

class AMAPlayerCharacter;
class UMAHighlightComponent;
class UPrimitiveComponent;
class UWidgetComponent;

// Usage from an owning actor:
// InteractComponent->CALL_SETUP_INTERACT(HandleInteract);
// Set the default WBP in MA Game Settings.
#define CALL_SETUP_INTERACT(MethodName) SetupInteraction(this, &std::remove_pointer_t<decltype(this)>::MethodName)

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UInteractComponent : public USphereComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UInteractComponent();

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
	
private:
	UPROPERTY(VisibleAnywhere, Category="MA|UI", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UWidgetComponent> InteractKeyWidgetComp;
	
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	TWeakObjectPtr<UMAHighlightComponent> HighlightComponent;
	TFunction<void(AMAPlayerCharacter*)> InteractionHandler;
	bool bFocused = false;
};
