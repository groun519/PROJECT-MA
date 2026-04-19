#include "LobbyAvatarSlot.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/WidgetComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/MAPlayerState.h"
#include "LobbyGameState.h"
#include "Framework/MAGameInstance.h"
#include "Widget/Lobby/Avatar/LobbyAvatarNameWidget.h"
#include "Widget/Lobby/Avatar/LobbyAvatarReadyWidget.h"
#include "LobbyAvatarAnimInstance.h"
#include "Player/Loadout/Data/LoadoutDataSet.h"
#include "Player/Loadout/Data/LoadoutWeaponData.h"
#include "Player/Loadout/Data/LoadoutEyeShapePresetData.h"
#include "Player/Mount/Data/MountData.h"
#include "Animation/AnimSequence.h"
#include "Engine/DataTable.h"

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

	/** Mount SKM **/
	MountMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MountMesh"));
	MountMesh->SetupAttachment(Root);
	MountMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MountMesh->SetGenerateOverlapEvents(false);
	MountMesh->SetCanEverAffectNavigation(false);

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
	MountMesh->SetVisibility(false, true);
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
	if (Occupant && LoadoutChangedHandle.IsValid())
	{
		Occupant->OnLoadoutChanged.Remove(LoadoutChangedHandle);
		LoadoutChangedHandle.Reset();
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
	if (ULobbyAvatarNameWidget* NameUI = Cast<ULobbyAvatarNameWidget>(NameWidget->GetUserWidgetObject()))
	{
		const FString NewText = Occupant ? Occupant->GetPlayerName() : TEXT("Empty");
		NameUI->SetNameText(NewText);
	}
	NameWidget->SetVisibility(Occupant != nullptr, true);

	ReadyWidget->SetVisibility(Occupant != nullptr, true);

	InviteWidget->SetVisibility(Occupant == nullptr, true);

	AvatarMesh->SetVisibility(Occupant != nullptr && !bMountPreviewVisible, true);
	WeaponMesh->SetVisibility(Occupant != nullptr && !bMountPreviewVisible && bWeaponPreviewVisible, true);
	const bool bShowMountPreview = Occupant != nullptr && bMountPreviewVisible && MountMesh->GetSkeletalMeshAsset() != nullptr;
	MountMesh->SetVisibility(bShowMountPreview, true);
	AvatarSpotLight->SetVisibility(Occupant != nullptr, true);

	if (Occupant)
	{
		LoadoutChangedHandle = Occupant->OnLoadoutChanged.AddUObject(
			this,
			&ALobbyAvatarSlot::HandleLoadoutChanged
		);
		HandleLoadoutChanged(Occupant->GetLoadoutSelection());
	}
	else
	{
		ApplyLoadoutMountId(NAME_None);
	}
}

void ALobbyAvatarSlot::HandleLoadoutChanged(const FLoadoutSelection& Loadout)
{
	ApplyLoadoutEyeShape(Loadout.EyeShapeId);
	ApplyLoadoutWeaponId(Loadout.WeaponId);
	ApplyLoadoutColor(Loadout.Color);
}

void ALobbyAvatarSlot::SetLocalHidden(bool bHide)
{
	SetActorHiddenInGame(bHide);
}

bool ALobbyAvatarSlot::EnsureAvatarDynMat()
{
	if (!AvatarDynMat)
	{
		AvatarDynMat = AvatarMesh->CreateAndSetMaterialInstanceDynamic(0);
	}
	return AvatarDynMat != nullptr;
}

void ALobbyAvatarSlot::ApplyLoadoutColor(const FMaterialParamDataPair& ColorData)
{
	if (!EnsureAvatarDynMat()) return;

	AvatarDynMat->SetVectorParameterValue("Body_Color", ColorData.BodyData.Color);
	AvatarDynMat->SetScalarParameterValue("Body_Emissive", ColorData.BodyData.Emissive);
	AvatarDynMat->SetVectorParameterValue("Eye_Color", ColorData.EyeData.Color);
	AvatarDynMat->SetScalarParameterValue("Eye_Emissive", ColorData.EyeData.Emissive);
	AvatarDynMat->SetScalarParameterValue("Radius_Inner", CurrentEyeShapeData.RadiusInner);
	AvatarDynMat->SetScalarParameterValue("Radius_Outter", CurrentEyeShapeData.RadiusOutter);
	AvatarDynMat->SetScalarParameterValue("Softness", CurrentEyeShapeData.Softness);
	AvatarDynMat->SetScalarParameterValue("Eye_Width", CurrentEyeShapeData.EyeWidth);
	AvatarDynMat->SetScalarParameterValue("Eye_Height", CurrentEyeShapeData.EyeHeight);
	AvatarDynMat->SetScalarParameterValue("_UseTexture", CurrentEyeShapeData.UseTexture);
	AvatarDynMat->SetTextureParameterValue("_EyeTexture", CurrentEyeShapeData.EyeTexture);
}

void ALobbyAvatarSlot::ApplyLoadoutEyeShape(FName EyeShapeId)
{
	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	const UDataTable* ResolvedEyeShapeDataTable = nullptr;
	if (LoadoutDataSet && LoadoutDataSet->EyeShapeDataTable)
	{
		ResolvedEyeShapeDataTable = LoadoutDataSet->EyeShapeDataTable;
	}

	CurrentEyeShapeData = FEyeShapeParamData();
	LoadoutEyeShapeTableUtils::ResolveEyeShapeData(ResolvedEyeShapeDataTable, EyeShapeId, CurrentEyeShapeData);

	if (!EnsureAvatarDynMat()) return;

	AvatarDynMat->SetScalarParameterValue("Radius_Inner", CurrentEyeShapeData.RadiusInner);
	AvatarDynMat->SetScalarParameterValue("Radius_Outter", CurrentEyeShapeData.RadiusOutter);
	AvatarDynMat->SetScalarParameterValue("Softness", CurrentEyeShapeData.Softness);
	AvatarDynMat->SetScalarParameterValue("Eye_Width", CurrentEyeShapeData.EyeWidth);
	AvatarDynMat->SetScalarParameterValue("Eye_Height", CurrentEyeShapeData.EyeHeight);
	AvatarDynMat->SetScalarParameterValue("_UseTexture", CurrentEyeShapeData.UseTexture);
	AvatarDynMat->SetTextureParameterValue("_EyeTexture", CurrentEyeShapeData.EyeTexture);
}

void ALobbyAvatarSlot::ApplyLoadoutMountId(FName MountId)
{
	if (MountId.IsNone())
	{
		MountMesh->SetSkeletalMesh(nullptr);
		MountMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MountMesh->SetAnimation(nullptr);
		SetMountPreviewVisible(bMountPreviewVisible);
		return;
	}

	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	const UDataTable* MountDataTable = LoadoutDataSet ? LoadoutDataSet->MountDataTable : nullptr;
	if (!MountDataTable)
	{
		MountMesh->SetSkeletalMesh(nullptr);
		MountMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MountMesh->SetAnimation(nullptr);
		SetMountPreviewVisible(bMountPreviewVisible);
		return;
	}

	const FMountDataRow* Row = MountDataTable->FindRow<FMountDataRow>(MountId, TEXT("LobbyAvatarSlotMountPreview"));
	if (!Row)
	{
		MountMesh->SetSkeletalMesh(nullptr);
		MountMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MountMesh->SetAnimation(nullptr);
		SetMountPreviewVisible(bMountPreviewVisible);
		return;
	}

	MountMesh->SetSkeletalMesh(Row->MountMesh.LoadSynchronous());
	MountMesh->SetAnimationMode(EAnimationMode::AnimationSingleNode);

	if (UAnimSequence* PreviewIdleAnimation = Row->PreviewIdleAnimation.LoadSynchronous())
	{
		MountMesh->PlayAnimation(PreviewIdleAnimation, true);
	}
	else
	{
		MountMesh->SetAnimation(nullptr);
	}

	SetMountPreviewVisible(bMountPreviewVisible);
}

void ALobbyAvatarSlot::SetMountPreviewVisible(bool bVisible)
{
	bMountPreviewVisible = bVisible;

	const bool bShowAvatarMeshes = Occupant != nullptr && !bMountPreviewVisible;
	AvatarMesh->SetVisibility(bShowAvatarMeshes, true);
	WeaponMesh->SetVisibility(bShowAvatarMeshes && bWeaponPreviewVisible, true);
	const bool bShowMountPreview = Occupant != nullptr && bMountPreviewVisible && MountMesh->GetSkeletalMeshAsset() != nullptr;
	MountMesh->SetVisibility(bShowMountPreview, true);
}

void ALobbyAvatarSlot::SetWeaponPreviewVisible(bool bVisible)
{
	bWeaponPreviewVisible = bVisible;
	const bool bShowWeapon = Occupant != nullptr && !bMountPreviewVisible && bWeaponPreviewVisible;
	WeaponMesh->SetVisibility(bShowWeapon, true);
}

void ALobbyAvatarSlot::SetWeaponOnlyOwnerSee(bool bEnable)
{
	WeaponMesh->SetOnlyOwnerSee(bEnable);
}

void ALobbyAvatarSlot::ApplyLoadoutWeaponId(FName WeaponId)
{
	if (WeaponId.IsNone())
	{
		WeaponMesh->SetSkeletalMesh(nullptr);
		return;
	}

	const UMAGameInstance* GI = GetGameInstance<UMAGameInstance>();
	const ULoadoutDataSet* LoadoutDataSet = GI ? GI->TryGetLoadoutDataSet() : nullptr;
	const UDataTable* ResolvedWeaponDataTable = nullptr;
	if (LoadoutDataSet && LoadoutDataSet->WeaponDataTable)
	{
		ResolvedWeaponDataTable = LoadoutDataSet->WeaponDataTable;
	}

	// Keep current preview mesh if no table is assigned.
	if (!ResolvedWeaponDataTable) return;

	const FLoadoutWeaponDataRow* Row = ResolvedWeaponDataTable->FindRow<FLoadoutWeaponDataRow>(WeaponId, TEXT("LobbyAvatarSlot"));
	if (!Row)
	{
		WeaponMesh->SetSkeletalMesh(nullptr);
		return;
	}

	USkeletalMesh* Mesh = Row->WeaponMesh.LoadSynchronous();
	WeaponMesh->SetSkeletalMesh(Mesh);
	WeaponMesh->SetRelativeTransform(Row->WeaponOffset);
}

void ALobbyAvatarSlot::ApplyLoadoutWeaponMesh(USkeletalMesh* Mesh, const FTransform& Offset)
{
	WeaponMesh->SetSkeletalMesh(Mesh);
	WeaponMesh->SetRelativeTransform(Offset);
	WeaponMesh->SetHiddenInGame(false);
	WeaponMesh->SetVisibility(Occupant != nullptr && !bMountPreviewVisible && bWeaponPreviewVisible, true);
	WeaponMesh->SetOnlyOwnerSee(false);
	WeaponMesh->SetOwnerNoSee(false);
}
void ALobbyAvatarSlot::SetLobbyState(ELobbyAvatarState State)
{
	if (!Occupant) return;

	if (ULobbyAvatarReadyWidget* ReadyUI = Cast<ULobbyAvatarReadyWidget>(ReadyWidget->GetUserWidgetObject()))
	{
		ReadyUI->SetLobbyState(State);
	}

	if (ULobbyAvatarAnimInstance* LobbyAnim = Cast<ULobbyAvatarAnimInstance>(AvatarMesh->GetAnimInstance()))
	{
		LobbyAnim->SetLobbyState(State);
	}

	const FLinearColor LightColor = (State == ELobbyAvatarState::Ready)
		? FLinearColor(0.1f, 0.9f, 0.1f, 1.0f)
		: FLinearColor::White;
	AvatarSpotLight->SetLightColor(LightColor);
}
