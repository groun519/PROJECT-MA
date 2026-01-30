// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyAvatarSlot.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/WidgetComponent.h"
#include "Player/MAPlayerState.h"
#include "LobbyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/Lobby/Avatar/LobbyAvatarNameWidget.h"
#include "Widget/Lobby/LobbyInviteWidget.h"

ALobbyAvatarSlot::ALobbyAvatarSlot()
{
	PrimaryActorTick.bCanEverTick = false;

	/** Root **/
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

    /** Avatar SKM **/
	AvatarMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AvatarMesh"));
	AvatarMesh->SetupAttachment(Root);

	/** Weapon SKM **/
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(AvatarMesh, WeaponSocketName);

	/** Spot Light **/
	AvatarSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("AvatarSpotLight"));
	AvatarSpotLight->SetupAttachment(Root);
	AvatarSpotLight->SetRelativeLocation(FVector(215.f, 0.f, 500.f));
	AvatarSpotLight->SetRelativeRotation(FRotator(0.f, -70.f, 180.f));
	AvatarSpotLight->SetMobility(EComponentMobility::Stationary);
	AvatarSpotLight->SetIntensity(100.f);
	AvatarSpotLight->SetLightColor(FLinearColor::White);
	AvatarSpotLight->SetInnerConeAngle(8.f);
	AvatarSpotLight->SetOuterConeAngle(10.f);
	AvatarSpotLight->SetSourceRadius(750.f);
	AvatarSpotLight->SetVisibility(false, true);

	/** Name Widget **/
	NameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidget"));
	NameWidget->SetupAttachment(Root);
	NameWidget->SetWidgetSpace(EWidgetSpace::Screen);
	NameWidget->SetDrawSize(FVector2D(300.f, 50.f));

	/** Invite Widget **/
	InviteWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InviteWidget"));
	InviteWidget->SetupAttachment(Root);
	InviteWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InviteWidget->SetDrawSize(FVector2D(200.f, 50.f));

	AvatarMesh->SetVisibility(false, true);
	WeaponMesh->SetVisibility(false, true);
	NameWidget->SetVisibility(false, true);
	InviteWidget->SetVisibility(false, true);
}

void ALobbyAvatarSlot::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (ALobbyGameState* LGS = World->GetGameState<ALobbyGameState>())
		{
			LGS->RegisterAvatarSlot(this);
		}
	}
}

void ALobbyAvatarSlot::SetOccupant(AMAPlayerState* NewPlayerState)
{
	Occupant = NewPlayerState;
	if (NameWidget)
	{
		if (NameWidgetClass)
		{
			NameWidget->SetWidgetClass(NameWidgetClass);
		}
		if (ULobbyAvatarNameWidget* NameUI = Cast<ULobbyAvatarNameWidget>(NameWidget->GetUserWidgetObject()))
		{
			const FString NewText = Occupant ? Occupant->GetPlayerName() : TEXT("Empty");
			NameUI->SetNameText(NewText);
		}
		NameWidget->SetVisibility(Occupant != nullptr, true);
	}
	if (InviteWidget)
	{
		if (InviteWidgetClass)
		{
			InviteWidget->SetWidgetClass(InviteWidgetClass);
		}
		InviteWidget->SetVisibility(Occupant == nullptr, true);
	}
	if (AvatarMesh)
	{
		AvatarMesh->SetVisibility(Occupant != nullptr, true);
	}
	if (WeaponMesh)
	{
		WeaponMesh->SetVisibility(Occupant != nullptr, true);
	}
	if (AvatarSpotLight)
	{
		AvatarSpotLight->SetVisibility(Occupant != nullptr, true);
	}
}

void ALobbyAvatarSlot::SetLocalHidden(bool bHide)
{
	const bool bVisible = !bHide;
	if (!bVisible)
	{
		if (AvatarMesh) { AvatarMesh->SetVisibility(false, true); }
		if (WeaponMesh) { WeaponMesh->SetVisibility(false, true); }
		if (NameWidget) { NameWidget->SetVisibility(false, true); }
		if (InviteWidget) { InviteWidget->SetVisibility(false, true); }
		if (AvatarSpotLight) { AvatarSpotLight->SetVisibility(false, true); }
		return;
	}

	SetOccupant(Occupant);
}
