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
	ApplyCurrentInteractFocus(Interactor);
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
	if (!bInteractionEnabled) return;

	if (UMAInteractableComponent* Comp = CurrentInteractableComponent.Get())
	{
		Comp->RequestInteract(Interactor);
	}
}

void UMAInteractorComponent::SetInteractionEnabled(bool bEnabled, AMAPlayerCharacter* Interactor)
{
	check(Interactor);
	if (bInteractionEnabled == bEnabled) return;

	bInteractionEnabled = bEnabled;
	ApplyCurrentInteractFocus(Interactor);
}

void UMAInteractorComponent::ApplyCurrentInteractFocus(AMAPlayerCharacter* Interactor)
{
	check(Interactor);

	if (UMAInteractableComponent* Comp = CurrentInteractableComponent.Get())
	{
		Comp->SetInteractFocused(Interactor, bInteractionEnabled);
	}
}


