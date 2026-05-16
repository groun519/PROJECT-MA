#include "Convenience/MAInteractorComponent.h"

#include "Convenience/MAInteractableComponent.h"
#include "Player/MAPlayerCharacter.h"

void UMAInteractorComponent::SetCurrentInteractableComponent(UMAInteractableComponent* NewComp, AMAPlayerCharacter* Interactor)
{
	check(NewComp);
	check(Interactor);
	if (CurrentInteractableComponent == NewComp) return;

	if (CurrentInteractableComponent.IsValid())
	{
		CurrentInteractableComponent->SetInteractFocused(Interactor, false);
	}

	CurrentInteractableComponent = NewComp;
	NewComp->SetInteractFocused(Interactor, true);
}

void UMAInteractorComponent::ClearCurrentInteractableComponent(UMAInteractableComponent* Comp, AMAPlayerCharacter* Interactor)
{
	check(Interactor);
	if (CurrentInteractableComponent.Get() != Comp) return;

	if (Comp) Comp->SetInteractFocused(Interactor, false);
	CurrentInteractableComponent = nullptr;
}

void UMAInteractorComponent::Interact(AMAPlayerCharacter* Interactor)
{
	check(Interactor);

	if (UMAInteractableComponent* Comp = CurrentInteractableComponent.Get())
	{
		Comp->RequestInteract(Interactor);
	}
}


