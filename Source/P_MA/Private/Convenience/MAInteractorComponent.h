#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MAInteractorComponent.generated.h"

class AMAPlayerCharacter;
class UMAInteractableComponent;

UCLASS()
class P_MA_API UMAInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void SetCurrentInteractableComponent(UMAInteractableComponent* NewComp, AMAPlayerCharacter* Interactor);
	void ClearCurrentInteractableComponent(UMAInteractableComponent* Comp, AMAPlayerCharacter* Interactor);
	void Interact(AMAPlayerCharacter* Interactor);
	void SetInteractionEnabled(bool bEnabled, AMAPlayerCharacter* Interactor);

private:
	void ApplyCurrentInteractFocus(AMAPlayerCharacter* Interactor);

	TWeakObjectPtr<UMAInteractableComponent> CurrentInteractableComponent;
	bool bInteractionEnabled = true;
};


