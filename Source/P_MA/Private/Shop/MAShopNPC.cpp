#include "Shop/MAShopNPC.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Convenience/MAInteractableComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
#include "GAS/Skill/Definition/MASkillDefinition.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerController.h"
#include "Player/MAPlayerControllerBase.h"
#include "Player/Camera/MAPlayerCameraDirectorComponent.h"
#include "Player/Components/MACurrencyComponent.h"
#include "Shop/MAShopProductFinder.h"
#include "Widget/Shop/MAShopWidget.h"
#include "Net/UnrealNetwork.h"

AMAShopNPC::AMAShopNPC()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractableComponent = CreateDefaultSubobject<UMAInteractableComponent>(TEXT("InteractableComponent"));
	RootComponent = InteractableComponent;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ShopCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("ShopCameraComponent"));
	ShopCameraComponent->SetupAttachment(RootComponent);
	ShopCameraComponent->SetRelativeLocation(FVector(-250.f, 0.f, 150.f));
	ShopCameraComponent->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));

	HighlightComponent = CreateDefaultSubobject<UMAHighlightComponent>(TEXT("HighlightComponent"));
	HighlightComponent->AddTarget(MeshComponent);
	InteractableComponent->CALL_SETUP_HIGHLIGHTER(HighlightComponent);
	InteractableComponent->CALL_SETUP_INTERACT(HandleInteract);

	FDirectoryPath ModuleRootPath;
	ModuleRootPath.Path = TEXT("/Game/_WorkSpace/GAS/Module");
	ModuleRootPaths.Add(ModuleRootPath);

	ModuleStockCountRange.Min = 3;
	ModuleStockCountRange.Max = 3;
}

void AMAShopNPC::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority()) RefreshStock();
}

void AMAShopNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMAShopNPC, CurrentStockEntries);
}

bool AMAShopNPC::RequestPurchase(APlayerController* PlayerController, int32 StockId)
{
	if (!HasAuthority()) return false;
	if (StockId == INDEX_NONE) return false;

	const int32 StockIndex = CurrentStockEntries.IndexOfByPredicate([StockId](const FMAShopStockEntry& Entry)
	{
		return Entry.StockId == StockId;
	});
	if (StockIndex == INDEX_NONE) return false;

	const FMAShopStockEntry Entry = CurrentStockEntries[StockIndex];
	if (!PlayerController || !Entry.SkillDefinition) return false;

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(PlayerController->GetPawn());
	if (!PlayerCharacter) return false;

	UMACurrencyComponent* Currency = PlayerCharacter->GetCurrencyComponent();
	if (!Currency || !Currency->HasCoin(Entry.Price)) return false;

	UMASkillModuleInventoryComponent* ModuleInventory = PlayerCharacter->GetSkillModuleInventoryComponent();
	if (!ModuleInventory || !ModuleInventory->RequestGrantModule(Entry.SkillDefinition)) return false;
	if (!Currency->TrySpendCoin(Entry.Price)) return false;

	CurrentStockEntries.RemoveAt(StockIndex);
	ForceNetUpdate();
	if (ActiveShopWidget) ActiveShopWidget->RefreshStock();
	return true;
}

void AMAShopNPC::RefreshStock()
{
	if (!HasAuthority()) return;

	CurrentStockEntries = GenerateShopStock();
	ForceNetUpdate();

	if (ActiveShopWidget) ActiveShopWidget->RefreshStock();
}

void AMAShopNPC::CloseShop(APlayerController* PlayerController)
{
	check(PlayerController);

	if (ActiveShopWidget)
	{
		ActiveShopWidget->RemoveFromParent();
		ActiveShopWidget = nullptr;
	}

	if (HiddenShopInteractor.IsValid())
	{
		PlayerController->HiddenActors.Remove(HiddenShopInteractor.Get());
		HiddenShopInteractor.Reset();
	}
	if (AMAPlayerController* MAPlayerController = Cast<AMAPlayerController>(PlayerController))
	{
		MAPlayerController->SetGameplayWidgetVisible(true);
	}

	if (AMAPlayerControllerBase* MAPlayerController = Cast<AMAPlayerControllerBase>(PlayerController))
	{
		if (UMAPlayerCameraDirectorComponent* CameraDirector = MAPlayerController->GetCameraDirector())
		{
			TWeakObjectPtr<UMAPlayerCameraDirectorComponent> WeakCameraDirector = CameraDirector;
			const float FadeInSeconds = ShopCameraFadeSettings.FadeInSeconds;
			CameraDirector->FadeOut(
				ShopCameraFadeSettings.FadeOutSeconds,
				[WeakCameraDirector, FadeInSeconds]()
				{
					if (!WeakCameraDirector.IsValid()) return;
					WeakCameraDirector->SwitchToPawnCamera();
					WeakCameraDirector->FadeIn(FadeInSeconds);
				}
			);
		}
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetHideCursorDuringCapture(false);
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

void AMAShopNPC::HandleInteract(AMAPlayerCharacter* Interactor)
{
	OpenShopFor(Interactor);
}

void AMAShopNPC::OpenShopFor(AMAPlayerCharacter* Interactor)
{
	if (!Interactor || !ShopWidgetClass) return;
	if (ActiveShopWidget && ActiveShopWidget->IsInViewport()) return;

	APlayerController* PlayerController = Cast<APlayerController>(Interactor->GetController());
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	ActiveShopWidget = CreateWidget<UMAShopWidget>(PlayerController, ShopWidgetClass);
	if (!ActiveShopWidget) return;

	ActiveShopWidget->AddToViewport(100);
	ActiveShopWidget->InitializeShop(this);

	if (AMAPlayerController* MAPlayerController = Cast<AMAPlayerController>(PlayerController))
	{
		MAPlayerController->SetGameplayWidgetVisible(false);
	}

	PlayerController->HiddenActors.AddUnique(Interactor);
	HiddenShopInteractor = Interactor;

	if (AMAPlayerControllerBase* MAPlayerController = Cast<AMAPlayerControllerBase>(PlayerController))
	{
		if (UMAPlayerCameraDirectorComponent* CameraDirector = MAPlayerController->GetCameraDirector())
		{
			TWeakObjectPtr<AMAShopNPC> WeakThis = this;
			TWeakObjectPtr<UMAPlayerCameraDirectorComponent> WeakCameraDirector = CameraDirector;
			const float FadeInSeconds = ShopCameraFadeSettings.FadeInSeconds;
			CameraDirector->FadeOut(
				ShopCameraFadeSettings.FadeOutSeconds,
				[WeakThis, WeakCameraDirector, FadeInSeconds]()
				{
					if (!WeakThis.IsValid() || !WeakCameraDirector.IsValid()) return;
					WeakCameraDirector->SwitchToViewTarget(WeakThis.Get());
					WeakCameraDirector->FadeIn(FadeInSeconds);
				}
			);
		}
	}

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ActiveShopWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

TArray<FMAShopStockEntry> AMAShopNPC::GenerateShopStock() const
{
	TArray<FMAShopStockEntry> Stock;

	TArray<UMASkillDefinition*> SkillDefinitions;
	FMAShopProductFinder::FindSkillModules(ModuleRootPaths, SkillDefinitions);
	const int32 Count = ModuleStockCountRange.ResolveCount();
	if (Count <= 0 || SkillDefinitions.IsEmpty()) return Stock;

	TArray<UMASkillDefinition*> Pool = SkillDefinitions;
	const int32 TargetCount = FMath::Min(Count, Pool.Num());

	for (int32 Index = 0; Index < TargetCount; ++Index)
	{
		const int32 PickIndex = FMath::RandRange(Index, Pool.Num() - 1);
		Pool.Swap(Index, PickIndex);

		if (!Pool[Index]) continue;

		FMAShopStockEntry Entry;
		Entry.StockId = Index;
		Entry.VisualSeed = FMath::Rand();
		Entry.SkillDefinition = Pool[Index];
		Entry.Price = ResolveModulePrice(Pool[Index]);
		Stock.Add(Entry);
	}
	return Stock;
}

int32 AMAShopNPC::ResolveModulePrice(const UMASkillDefinition* SkillDefinition) const
{
	if (!SkillDefinition) return 0;
	check(ModuleQualityData);
	return ModuleQualityData->ResolvePrice(SkillDefinition->GetModuleQuality());
}

void AMAShopNPC::OnRep_CurrentStockEntries()
{
	if (ActiveShopWidget) ActiveShopWidget->RefreshStock();
}
