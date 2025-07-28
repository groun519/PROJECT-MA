// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/MACharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAttributeSet.h"
#include "Kismet/GameplayStatics.h"

AMACharacter::AMACharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	MAAbilitySystemComponent = CreateDefaultSubobject<UMAAbilitySystemComponent>("MAAbility System Component");
	MAAttributeSet = CreateDefaultSubobject<UMAAttributeSet>("MAAttribute Set");
}

void AMACharacter::ServerSideInit()
{
	MAAbilitySystemComponent->InitAbilityActorInfo(this, this);
	MAAbilitySystemComponent->ApplyInitialEffects();
	MAAbilitySystemComponent->GiveInitialAbilities();
}

void AMACharacter::ClientSideInit()
{
	MAAbilitySystemComponent->InitAbilityActorInfo(this, this);
}

bool AMACharacter::IsLocallyControlledByPlayer() const
{
	return GetController() && GetController()->IsLocalPlayerController();
}

void AMACharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AMACharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AMACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}

UAbilitySystemComponent* AMACharacter::GetAbilitySystemComponent() const
{
	return MAAbilitySystemComponent;
}

void AMACharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

