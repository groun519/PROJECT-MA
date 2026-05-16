#include "MAInteractableComponent.h"

#include "Components/WidgetComponent.h"
#include "Convenience/MAInteractorComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "GameFramework/PlayerController.h"
#include "Player/MAPlayerCharacter.h"
#include "P_MA/P_MA.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Input/MAInputKeyPromptWidget.h"

UMAInteractableComponent::UMAInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Hitbox, ECR_Overlap);
	InitSphereRadius(150.0f);

	InteractKeyWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractKeyWidgetComp"));
	InteractKeyWidgetComp->SetupAttachment(this);
	InteractKeyWidgetComp->SetVisibility(false);
	InteractKeyWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	InteractKeyWidgetComp->SetDrawAtDesiredSize(true);
}

void UMAInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	const UMAGameSettings* GameSettings = UMAGameSettings::Get();
	check(GameSettings);

	UClass* InteractKeyWidgetClass = GameSettings->DefaultInteractKeyWidgetClass.LoadSynchronous();
	checkf(InteractKeyWidgetClass, TEXT("Set DefaultInteractKeyWidgetClass in MA Game Settings."));

	InteractKeyWidgetComp->SetWidgetClass(InteractKeyWidgetClass);
	InteractKeyWidgetComp->InitWidget();
	CastChecked<UMAInputKeyPromptWidget>(InteractKeyWidgetComp->GetUserWidgetObject());

	InteractKeyWidgetComp->SetRelativeLocation(FVector::ZeroVector);
	InteractKeyWidgetComp->SetVisibility(false);

	OnComponentBeginOverlap.AddDynamic(this, &UMAInteractableComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UMAInteractableComponent::HandleEndOverlap);
}

void UMAInteractableComponent::RequestInteract(AMAPlayerCharacter* Interactor)
{
	if (InteractionHandler) InteractionHandler(Interactor);
}

void UMAInteractableComponent::SetInteractFocused(AMAPlayerCharacter* Interactor, bool bNewFocused)
{
	if (bFocused == bNewFocused) return;
	bFocused = bNewFocused;

	/** Key Widget **/
	UMAInputKeyPromptWidget* KeyPromptWidget =
		CastChecked<UMAInputKeyPromptWidget>(InteractKeyWidgetComp->GetUserWidgetObject());
	if (bFocused)
	{
		KeyPromptWidget->SetInputContext(
			Cast<APlayerController>(Interactor->GetController()),
			Interactor->GetGameplayInputMappingContext());
	}
	else
	{
		KeyPromptWidget->ClearInputContext();
	}
	InteractKeyWidgetComp->SetVisibility(bFocused);

	/** Highlight **/
	if (UMAHighlightComponent* Highlighter = HighlightComponent.Get())
	{
		Highlighter->SetHighlighted(bFocused);
	}
}

void UMAInteractableComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(OtherActor))
	{
		if (!Player->IsLocallyControlled()) return;
		Player->GetInteractorComponent()->SetCurrentInteractableComponent(this, Player);
	}
}

void UMAInteractableComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(OtherActor))
	{
		if (!Player->IsLocallyControlled()) return;
		Player->GetInteractorComponent()->ClearCurrentInteractableComponent(this, Player);
	}
}


