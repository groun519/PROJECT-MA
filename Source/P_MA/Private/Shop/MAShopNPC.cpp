#include "Shop/MAShopNPC.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Convenience/MAInteractableComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "Convenience/MAInteractorComponent.h"
#include "Engine/AssetManager.h"
#include "Framework/MAGameMode.h"
#include "Inventory/MAInventoryComponent.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerController.h"
#include "Player/MAPlayerControllerBase.h"
#include "Player/Camera/MAPlayerCameraDirectorComponent.h"
#include "Player/Components/MACurrencyComponent.h"
#include "Shop/MAShopModulePool.h"
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

	ModuleStockCountRange.Min = 3;
	ModuleStockCountRange.Max = 3;
	ItemStockCountRange.Min = 3;
	ItemStockCountRange.Max = 3;
}

void AMAShopNPC::BeginPlay()
{
	Super::BeginPlay();
	SetTemporaryShopVisible(bTemporaryShopVisible);
	if (!HasAuthority()) return;

	RefreshStock();

	if (AMAGameMode* GameMode = GetWorld()->GetAuthGameMode<AMAGameMode>())
	{
		GameMode->OnMASectorStateChanged.RemoveAll(this);
		GameMode->OnMASectorStateChanged.AddUObject(this, &AMAShopNPC::HandleSectorStateChanged);
		SetTemporaryShopVisible(GameMode->GetMASectorState() == TemporaryVisibleState);
	}
}

void AMAShopNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (AMAGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AMAGameMode>() : nullptr)
		{
			GameMode->OnMASectorStateChanged.RemoveAll(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AMAShopNPC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMAShopNPC, CurrentProducts);
	DOREPLIFETIME(AMAShopNPC, bTemporaryShopVisible);
}

bool AMAShopNPC::Purchase(APlayerController* PlayerController, int32 StockId)
{
	if (!HasAuthority()) return false;
	if (StockId == INDEX_NONE) return false;

	const int32 StockIndex = CurrentProducts.IndexOfByPredicate([StockId](const FMAShopProduct& Product)
	{
		return Product.StockId == StockId;
	});
	if (StockIndex == INDEX_NONE) return false;

	const FMAShopProduct Product = CurrentProducts[StockIndex];
	if (!PlayerController || !Product.Module) return false;

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(PlayerController->GetPawn());
	if (!PlayerCharacter) return false;

	UMACurrencyComponent* Currency = PlayerCharacter->GetCurrencyComponent();
	UMAInventoryComponent* Inventory = PlayerCharacter->GetInventoryComponent();
	if (!Currency || !Inventory || !Currency->TrySpendCoin(Product.Price)) return false;
	if (!Inventory->RequestAddModule(Product.Module->GetModuleId()))
	{
		Currency->AddCoin(Product.Price);
		return false;
	}

	CurrentProducts.RemoveAt(StockIndex);
	ForceNetUpdate();
	if (ActiveShopWidget) ActiveShopWidget->RefreshStock();
	return true;
}

void AMAShopNPC::RefreshStock()
{
	if (!HasAuthority()) return;

	CurrentProducts = GenerateStock();
	ForceNetUpdate();

	if (ActiveShopWidget) ActiveShopWidget->RefreshStock();
}

void AMAShopNPC::SetStockCountsForTest(int32 Count)
{
	if (!HasAuthority()) return;

	const int32 ClampedCount = FMath::Max(0, Count);
	ModuleStockCountRange.Min = ClampedCount;
	ModuleStockCountRange.Max = ClampedCount;
	ItemStockCountRange.Min = ClampedCount;
	ItemStockCountRange.Max = ClampedCount;
}

void AMAShopNPC::HandleSectorStateChanged(EMASectorState NewState)
{
	if (NewState == RefreshStockState) RefreshStock();
	SetTemporaryShopVisible(NewState == TemporaryVisibleState);
}

void AMAShopNPC::SetTemporaryShopVisible(bool bVisible)
{
	if (HasAuthority() && bTemporaryShopVisible != bVisible)
	{
		bTemporaryShopVisible = bVisible;
		ForceNetUpdate();
	}

	SetActorHiddenInGame(!bVisible);
	if (InteractableComponent)
	{
		InteractableComponent->SetCollisionEnabled(bVisible ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}
	if (!bVisible)
	{
		if (HighlightComponent && InteractableComponent)
		{
			HighlightComponent->SetHighlight(*InteractableComponent, false);
		}
		if (ActiveShopWidget)
		{
			CloseShop(ActiveShopWidget->GetOwningPlayer());
		}
	}
}

void AMAShopNPC::OnRep_TemporaryShopVisible()
{
	SetTemporaryShopVisible(bTemporaryShopVisible);
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
		AMAPlayerCharacter* ShopInteractor = HiddenShopInteractor.Get();
		if (UMAInteractorComponent* InteractorComponent = ShopInteractor->GetInteractorComponent())
		{
			InteractorComponent->SetInteractionEnabled(true, ShopInteractor);
		}
		PlayerController->HiddenActors.Remove(ShopInteractor);
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
	if (!bTemporaryShopVisible) return;
	OpenShopFor(Interactor);
}

void AMAShopNPC::OpenShopFor(AMAPlayerCharacter* Interactor)
{
	if (!Interactor || !ShopWidgetClass) return;
	if (ActiveShopWidget && ActiveShopWidget->IsInViewport()) return;

	APlayerController* PlayerController = Cast<APlayerController>(Interactor->GetController());
	if (!PlayerController || !PlayerController->IsLocalController()) return;

	if (PlayerController->PlayerInput)
	{
		PlayerController->PlayerInput->FlushPressedKeys();
	}

	ActiveShopWidget = CreateWidget<UMAShopWidget>(PlayerController, ShopWidgetClass);
	if (!ActiveShopWidget) return;

	if (UMAInteractorComponent* InteractorComponent = Interactor->GetInteractorComponent())
	{
		InteractorComponent->SetInteractionEnabled(false, Interactor);
	}

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
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

TArray<FMAShopProduct> AMAShopNPC::GenerateStock() const
{
	TArray<FMAShopProduct> Stock;

	const int32 ModuleCount = ModuleStockCountRange.ResolveCount();
	const int32 ItemCount = ItemStockCountRange.ResolveCount();
	if ((ModuleCount <= 0 && ItemCount <= 0) || !ModulePool) return Stock;

	TArray<UMASkillModule*> ModuleCandidates;
	TArray<UMASkillModule*> ItemCandidates;
	UAssetManager& AssetManager = UAssetManager::Get();
	for (const int32 ModuleId : ModulePool->GetModuleIds())
	{
		const FSoftObjectPath ModulePath = AssetManager.GetPrimaryAssetPath(
			UMASkillModule::MakePrimaryAssetId(ModuleId));
		UMASkillModule* Module = Cast<UMASkillModule>(ModulePath.TryLoad());
		if (!Module) continue;

		switch (Module->GetModuleType())
		{
		case EMASkillModuleType::Module:
			ModuleCandidates.Add(Module);
			break;
		case EMASkillModuleType::Item:
			ItemCandidates.Add(Module);
			break;
		default:
			break;
		}
	}

	auto AppendRandomProducts = [this, &Stock](TArray<UMASkillModule*>& Candidates, const int32 Count)
	{
		const int32 ProductCount = FMath::Min(Count, Candidates.Num());
		for (int32 Index = 0; Index < ProductCount; ++Index)
		{
			Candidates.Swap(Index, FMath::RandRange(Index, Candidates.Num() - 1));
			UMASkillModule* Module = Candidates[Index];

			FMAShopProduct Product;
			Product.StockId = Stock.Num();
			Product.VisualSeed = FMath::Rand();
			Product.Module = Module;
			Product.Price = ResolvePrice(Module);
			Stock.Add(Product);
		}
	};
	AppendRandomProducts(ModuleCandidates, ModuleCount);
	AppendRandomProducts(ItemCandidates, ItemCount);
	return Stock;
}

int32 AMAShopNPC::ResolvePrice(const UMASkillModule* Module) const
{
	if (!Module) return 0;
	check(ModuleQualityData);
	return ModuleQualityData->ResolvePrice(Module->GetModuleQuality());
}

void AMAShopNPC::OnRep_CurrentProducts()
{
	if (ActiveShopWidget) ActiveShopWidget->RefreshStock();
}
