#include "Inventory/MAModuleDrop.h"

#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "Convenience/MAInteractableComponent.h"
#include "GAS/Skill/Addon/Item/MASkillModuleItemAddon.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "Inventory/MAInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/MAPlayerCharacter.h"
#include "Setting/MAGameSettings.h"
#include "Widget/Skill/MASkillTooltipWidget.h"

AMAModuleDrop::AMAModuleDrop()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractableComponent = CreateDefaultSubobject<UMAInteractableComponent>(TEXT("InteractableComponent"));
	RootComponent = InteractableComponent;
	InteractableComponent->CALL_SETUP_INTERACT(HandleInteract);
	InteractableComponent->CALL_SETUP_FOCUS(HandleFocus);
	InteractableComponent->CALL_SETUP_INTERACTION_MODE(Server);

	DropMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DropMeshComponent"));
	DropMeshComponent->SetupAttachment(RootComponent);
	DropMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HighlightComponent = CreateDefaultSubobject<UMAHighlightComponent>(TEXT("HighlightComponent"));
	HighlightComponent->AddTarget(DropMeshComponent);
	InteractableComponent->CALL_SETUP_HIGHLIGHTER(HighlightComponent);

	RarityVisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RarityVisualComponent"));
	RarityVisualComponent->SetupAttachment(RootComponent);
	RarityVisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TooltipWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TooltipWidgetComponent"));
	TooltipWidgetComponent->SetupAttachment(RootComponent);
	TooltipWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	TooltipWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	TooltipWidgetComponent->SetDrawAtDesiredSize(true);
	TooltipWidgetComponent->SetVisibility(false);
}

void AMAModuleDrop::BeginPlay()
{
	Super::BeginPlay();
	TooltipWidgetComponent->SetWidgetClass(TooltipWidgetClass);
	TooltipWidgetComponent->InitWidget();
	RefreshPresentation();
}

void AMAModuleDrop::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMAModuleDrop, DropData);
}

bool AMAModuleDrop::InitializeDrop(const int32 ModuleId, const int32 Count)
{
	if (!HasAuthority() || ModuleId <= 0 || Count <= 0) return false;

	CachedModule = UMASkillModule::LoadById(ModuleId);
	if (!CachedModule) return false;

	DropData.ModuleId = ModuleId;
	DropData.Count = Count;
	RefreshPresentation();
	ForceNetUpdate();
	return true;
}

void AMAModuleDrop::HandleInteract(AMAPlayerCharacter* Interactor)
{
	if (!HasAuthority() || bPickupInProgress || !Interactor || !DropData.IsValid()) return;

	UMAInventoryComponent* Inventory = Interactor->GetInventoryComponent();
	if (!Inventory) return;

	bPickupInProgress = true;
	if (Inventory->RequestAddModule(DropData.ModuleId, DropData.Count))
	{
		Destroy();
		return;
	}
	bPickupInProgress = false;
}

void AMAModuleDrop::HandleFocus(AMAPlayerCharacter*, const bool bFocused)
{
	TooltipWidgetComponent->SetVisibility(
		bFocused && CachedModule && TooltipWidgetComponent->GetUserWidgetObject());
}

void AMAModuleDrop::RefreshPresentation()
{
	if (!DropData.IsValid())
	{
		CachedModule = nullptr;
	}
	else if (!CachedModule || CachedModule->GetModuleId() != DropData.ModuleId)
	{
		CachedModule = UMASkillModule::LoadById(DropData.ModuleId);
	}
	InteractableComponent->SetCollisionEnabled(CachedModule
		? ECollisionEnabled::QueryOnly
		: ECollisionEnabled::NoCollision);
	if (GetNetMode() == NM_DedicatedServer) return;

	DropMeshComponent->SetStaticMesh(ResolveDropMesh());
	RefreshTooltip();

	const UMAModuleQualityData* QualityData = UMAGameSettings::Get()->GetModuleQualityData();
	const FMAModuleRarityData* RarityData = CachedModule && QualityData
		? QualityData->FindRarityData(CachedModule->GetModuleQuality().Rarity)
		: nullptr;
	RarityVisualComponent->SetVisibility(RarityData != nullptr);
	if (!RarityData) return;

	static const FName ColorParameterName(TEXT("RarityColor"));
	static const FName IntensityParameterName(TEXT("RarityIntensity"));
	RarityVisualComponent->SetColorParameterValueOnMaterials(ColorParameterName, RarityData->Color);
	RarityVisualComponent->SetScalarParameterValueOnMaterials(IntensityParameterName, RarityData->GlowAlpha);
}

void AMAModuleDrop::RefreshTooltip()
{
	UMASkillTooltipWidget* TooltipWidget =
		Cast<UMASkillTooltipWidget>(TooltipWidgetComponent->GetUserWidgetObject());
	if (!TooltipWidget) return;

	FMADisplayData DisplayData = CachedModule
		? CachedModule->ResolveDisplayData(UMAGameSettings::Get()->GetModuleQualityData())
		: FMADisplayData();
	if (DropData.Count > 1)
	{
		DisplayData.DisplayName = FText::Format(
			NSLOCTEXT("MASkillTooltipWidget", "ItemCountFormat", "{0} x{1}"),
			DisplayData.DisplayName,
			FText::AsNumber(DropData.Count));
	}
	TooltipWidget->SetDisplayData(DisplayData);
}

UStaticMesh* AMAModuleDrop::ResolveDropMesh() const
{
	if (!CachedModule) return nullptr;

	switch (CachedModule->GetModuleType())
	{
	case EMASkillModuleType::Module:
		return ModuleMesh;
	case EMASkillModuleType::Sub:
		return SubModuleMesh;
	case EMASkillModuleType::Item:
		if (const UMASkillModuleItemAddon* ItemAddon =
			CachedModule->FindAddon<UMASkillModuleItemAddon>())
		{
			if (UStaticMesh* ItemMesh = ItemAddon->GetWorldMesh()) return ItemMesh;
		}
		return DefaultItemMesh;
	default:
		return nullptr;
	}
}

void AMAModuleDrop::OnRep_DropData()
{
	RefreshPresentation();
}
