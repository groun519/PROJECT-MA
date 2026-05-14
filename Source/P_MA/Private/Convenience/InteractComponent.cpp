#include "InteractComponent.h"

#include "Components/WidgetComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "GameFramework/PlayerController.h"
#include "Player/MAPlayerCharacter.h"
#include "P_MA/P_MA.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Input/MAInputKeyPromptWidget.h"

UInteractComponent::UInteractComponent()
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

void UInteractComponent::BeginPlay()
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
	HighlightComponent = GetOwner() ? GetOwner()->FindComponentByClass<UMAHighlightComponent>() : nullptr;

	OnComponentBeginOverlap.AddDynamic(this, &UInteractComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UInteractComponent::HandleEndOverlap);
}

void UInteractComponent::RequestInteract(AMAPlayerCharacter* Interactor)
{
	if (InteractionHandler) InteractionHandler(Interactor);
}

void UInteractComponent::SetInteractFocused(AMAPlayerCharacter* Interactor, bool bNewFocused)
{
	if (bFocused == bNewFocused) return;

	UMAInputKeyPromptWidget* KeyPromptWidget = CastChecked<UMAInputKeyPromptWidget>(InteractKeyWidgetComp->GetUserWidgetObject());
	if (bNewFocused)
	{
		check(Interactor);
		KeyPromptWidget->SetInputAction(
			Cast<APlayerController>(Interactor->GetController()),
			Interactor->GetGameplayInputMappingContext(),
			Interactor->GetInteractInputAction());
	}
	else
	{
		KeyPromptWidget->ClearInputAction();
	}

	bFocused = bNewFocused;
	InteractKeyWidgetComp->SetVisibility(bFocused);

	if (UMAHighlightComponent* Highlighter = HighlightComponent.Get())
	{
		Highlighter->SetHighlighted(bFocused);
	}
}

void UInteractComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(OtherActor))
	{
		if (!Player->IsLocallyControlled()) return;
		Player->SetCurrentInteractComp(this);
		SetInteractFocused(Player, true);
	}
}

void UInteractComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AMAPlayerCharacter* Player = Cast<AMAPlayerCharacter>(OtherActor))
	{
		if (!Player->IsLocallyControlled()) return;
		Player->ClearCurrentInteractComp(this);
	}
}
