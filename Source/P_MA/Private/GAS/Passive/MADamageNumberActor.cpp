// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Passive/MADamageNumberActor.h"

#include "Components/WidgetComponent.h"
#include "Widget/MADamageTextWidget.h"

// Sets default values
AMADamageNumberActor::AMADamageNumberActor()
{
	PrimaryActorTick.bCanEverTick = false;

	InitialLifeSpan = 1.5f;
	
	DamageWidgetComp=CreateDefaultSubobject<UWidgetComponent>("DamageWidget");
	DamageWidgetComp->SetupAttachment(GetRootComponent());
	
	DamageWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
}

// Called when the game starts or when spawned
void AMADamageNumberActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMADamageNumberActor::PlayDamageText(float DamageAmount, bool bIsCritical, bool bIsPlayerHit)
{
	if (DamageWidgetComp)
	{
		if (UMADamageTextWidget* DamageWidget = Cast<UMADamageTextWidget>(DamageWidgetComp->GetUserWidgetObject()))
		{
			DamageWidget->SetDamageText(DamageAmount, bIsCritical, bIsPlayerHit);
		}
	}
}

