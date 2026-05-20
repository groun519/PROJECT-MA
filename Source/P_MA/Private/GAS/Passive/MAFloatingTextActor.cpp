#include "GAS/Passive/MAFloatingTextActor.h"

#include "Components/WidgetComponent.h"
#include "Widget/MAFloatingTextWidget.h"

AMAFloatingTextActor::AMAFloatingTextActor()
{
	PrimaryActorTick.bCanEverTick = false;

	InitialLifeSpan = 1.5f;
	
	FloatingTextWidgetComp = CreateDefaultSubobject<UWidgetComponent>("DamageWidget");
	RootComponent = FloatingTextWidgetComp;
	
	FloatingTextWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

void AMAFloatingTextActor::PlayText(const FText& Text, const FLinearColor& Color)
{
	if (FloatingTextWidgetComp)
	{
		if (UMAFloatingTextWidget* FloatingTextWidget = Cast<UMAFloatingTextWidget>(FloatingTextWidgetComp->GetUserWidgetObject()))
		{
			FloatingTextWidget->SetDisplayText(Text, Color);
		}
	}
}
