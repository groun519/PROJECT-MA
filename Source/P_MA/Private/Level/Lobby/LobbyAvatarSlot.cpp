// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyAvatarSlot.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/WidgetComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/MAPlayerState.h"
#include "LobbyGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/Lobby/Avatar/LobbyAvatarNameWidget.h"
#include "Widget/Lobby/Avatar/LobbyAvatarReadyWidget.h"
#include "Widget/Lobby/LobbyInviteWidget.h"
#include "LobbyAvatarAnimInstance.h"

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

	/** Ready Widget **/
	ReadyWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("ReadyWidget"));
	ReadyWidget->SetupAttachment(Root);
	ReadyWidget->SetWidgetSpace(EWidgetSpace::Screen);
	ReadyWidget->SetDrawSize(FVector2D(200.f, 50.f));

	/** Invite Widget **/
	InviteWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InviteWidget"));
	InviteWidget->SetupAttachment(Root);
	InviteWidget->SetWidgetSpace(EWidgetSpace::Screen);
	InviteWidget->SetDrawSize(FVector2D(200.f, 50.f));

	AvatarMesh->SetVisibility(false, true);
	WeaponMesh->SetVisibility(false, true);
	NameWidget->SetVisibility(false, true);
	ReadyWidget->SetVisibility(false, true);
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
	if (Occupant && LoadoutColorChangedHandle.IsValid())
	{
		Occupant->OnLoadoutColorChanged.Remove(LoadoutColorChangedHandle);
		LoadoutColorChangedHandle.Reset();
	}

	Occupant = NewPlayerState;
	if (AActor* NewOwner = Occupant ? Occupant->GetOwner() : nullptr)
	{
		SetOwner(NewOwner);
	}
	else if (!Occupant)
	{
		SetOwner(nullptr);
	}
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
	if (ReadyWidget)
	{
		if (ReadyWidgetClass)
		{
			ReadyWidget->SetWidgetClass(ReadyWidgetClass);
		}
		ReadyWidget->SetVisibility(Occupant != nullptr, true);
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

	if (Occupant)
	{
		LoadoutColorChangedHandle = Occupant->OnLoadoutColorChanged.AddUObject(
			this,
			&ALobbyAvatarSlot::ApplyLoadoutColor
		);
		ApplyLoadoutColor(Occupant->GetLoadoutColor());
	}
}

void ALobbyAvatarSlot::SetLocalHidden(bool bHide)
{
	SetActorHiddenInGame(bHide);
}

void ALobbyAvatarSlot::ApplyLoadoutColor(const FMaterialParamDataPair& ColorData)
{
	if (!AvatarMesh)
	{
		return;
	}

	if (!AvatarDynMat)
	{
		AvatarDynMat = AvatarMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	if (!AvatarDynMat)
	{
		return;
	}

	AvatarDynMat->SetVectorParameterValue("Body_Color", ColorData.BodyData.Color);
	AvatarDynMat->SetScalarParameterValue("Body_Emissive", ColorData.BodyData.Emissive);
	AvatarDynMat->SetVectorParameterValue("Eye_Color", ColorData.EyeData.Color);
	AvatarDynMat->SetScalarParameterValue("Eye_Emissive", ColorData.EyeData.Emissive);
}

void ALobbyAvatarSlot::SetWeaponOnlyOwnerSee(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetOnlyOwnerSee(bEnable);
	}
}

void ALobbyAvatarSlot::SetLobbyState(ELobbyAvatarState State)
{
	if (!ReadyWidget || !Occupant)
	{
		return;
	}

	if (ULobbyAvatarReadyWidget* ReadyUI = Cast<ULobbyAvatarReadyWidget>(ReadyWidget->GetUserWidgetObject()))
	{
		ReadyUI->SetLobbyState(State);
	}

	if (AvatarMesh)
	{
		if (ULobbyAvatarAnimInstance* LobbyAnim = Cast<ULobbyAvatarAnimInstance>(AvatarMesh->GetAnimInstance()))
		{
			LobbyAnim->SetLobbyState(State);
		}
	}

	if (AvatarSpotLight)
	{
		const FLinearColor LightColor = (State == ELobbyAvatarState::Ready)
			? FLinearColor(0.1f, 0.9f, 0.1f, 1.0f)
			: FLinearColor::White;
		AvatarSpotLight->SetLightColor(LightColor);
	}
}
