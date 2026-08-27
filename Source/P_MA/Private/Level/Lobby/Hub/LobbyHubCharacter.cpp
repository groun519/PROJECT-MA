#include "Level/Lobby/Hub/LobbyHubCharacter.h"

#include "AbilitySystemComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/MAAttributeSet.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Widget/MAOverHeadStatsGauge.h"

ALobbyHubCharacter::ALobbyHubCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bEnableMinimapCapture = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ALobbyHubCharacter::BeginPlay()
{
	Super::BeginPlay();

	// The inherited stimuli source is not part of the Hub character's role.
	if (UAIPerceptionStimuliSourceComponent* StimuliSource = FindComponentByClass<UAIPerceptionStimuliSourceComponent>())
	{
		StimuliSource->UnregisterFromSense(UAISense_Sight::StaticClass());
	}

	// Identify the inherited status gauge by its widget type so other widget components remain untouched.
	TInlineComponentArray<UWidgetComponent*> WidgetComponents(this);
	for (UWidgetComponent* WidgetComponent : WidgetComponents)
	{
		if (Cast<UMAOverHeadStatsGauge>(WidgetComponent->GetUserWidgetObject()))
		{
			WidgetComponent->SetHiddenInGame(true);
			break;
		}
	}
}

void ALobbyHubCharacter::InitializeHubRuntime()
{
	if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
	{
		// Do not call ServerSideInit(). The Hub owns no combat stats, effects, or granted abilities.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AbilitySystemComponent->SetLooseGameplayTagCount(UMAAbilitySystemStatics::GetAbilityBlockTag(), 1);
		if (!HasAuthority()) return;

		AbilitySystemComponent->SetNumericAttributeBase(UMAAttributeSet::GetMoveSpeedAttribute(), HubMoveSpeed);
		AbilitySystemComponent->SetNumericAttributeBase(UMAAttributeSet::GetSlowMultiplierAttribute(), 1.f);
	}
}
