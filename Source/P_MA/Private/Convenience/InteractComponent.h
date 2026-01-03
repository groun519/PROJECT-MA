#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "InteractComponent.generated.h"

class AMAPlayerCharacter;
class UWidgetComponent;

/** * How to Use ? 
 * 1. CALL_SETUP_INTERACT(MethodName) -> Connect logic without 'this'
 * 2. Fill in widgetcomp in Details Panel
 */
#define CALL_SETUP_INTERACT(MethodName) SetupInteraction(this, &std::remove_pointer_t<decltype(this)>::MethodName)

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class P_MA_API UInteractComponent : public USphereComponent
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	UInteractComponent();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MA|UI")
	TObjectPtr<UWidgetComponent> InteractKeyWidgetComp;

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

	UFUNCTION(BlueprintCallable, Category="MA|Interact")
	void RequestInteract(AMAPlayerCharacter* Interactor);

	void SetActive(bool bNewActive);
	
private:
	
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	TFunction<void(AMAPlayerCharacter*)> InteractionHandler;
	bool bActive = false;
};