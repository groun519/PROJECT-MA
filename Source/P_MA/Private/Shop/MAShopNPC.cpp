#include "Shop/MAShopNPC.h"

#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Convenience/MAInteractableComponent.h"
#include "Convenience/MAHighlightComponent.h"
#include "Convenience/MAInteractorComponent.h"
#include "Engine/AssetManager.h"
#include "Framework/MAGameMode.h"
#include "GAS/Skill/MASkillModuleInventoryComponent.h"
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

	DOREPLIFETIME(AMAShopNPC, CurrentStockEntries);
	DOREPLIFETIME(AMAShopNPC, bTemporaryShopVisible);
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
	if (!PlayerController || !Entry.SkillModule) return false;

	AMAPlayerCharacter* PlayerCharacter = Cast<AMAPlayerCharacter>(PlayerController->GetPawn());
	if (!PlayerCharacter) return false;

	UMACurrencyComponent* Currency = PlayerCharacter->GetCurrencyComponent();
	if (!Currency || !Currency->HasCoin(Entry.Price)) return false;

	UMASkillModuleInventoryComponent* ModuleInventory = PlayerCharacter->GetSkillModuleInventoryComponent();
	if (!ModuleInventory || !ModuleInventory->RequestGrantModule(Entry.SkillModule)) return false;
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

void AMAShopNPC::SetModuleStockCountForTest(int32 Count)
{
	if (!HasAuthority()) return;

	const int32 ClampedCount = FMath::Max(0, Count);
	ModuleStockCountRange.Min = ClampedCount;
	ModuleStockCountRange.Max = ClampedCount;
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
		if (HighlightComponent)
		{
			HighlightComponent->SetHighlighted(false);
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

TArray<FMAShopStockEntry> AMAShopNPC::GenerateShopStock() const
{
	TArray<FMAShopStockEntry> Stock;

	const int32 Count = ModuleStockCountRange.ResolveCount();
	if (Count <= 0 || !ModulePool) return Stock;

	TArray<int32> CandidateModuleIds = ModulePool->GetModuleIds();
	UAssetManager& AssetManager = UAssetManager::Get();
	for (int32 Index = 0; Index < CandidateModuleIds.Num() && Stock.Num() < Count; ++Index)
	{
		const int32 PickIndex = FMath::RandRange(Index, CandidateModuleIds.Num() - 1);
		CandidateModuleIds.Swap(Index, PickIndex);

		const int32 ModuleId = CandidateModuleIds[Index];
		const FSoftObjectPath ModulePath = AssetManager.GetPrimaryAssetPath(
			UMASkillModule::MakePrimaryAssetId(ModuleId));
		UMASkillModule* SkillModule = Cast<UMASkillModule>(ModulePath.TryLoad());
		if (!SkillModule) continue;
		if (static_cast<uint8>(SkillModule->GetModuleQuality().Rarity)
			> static_cast<uint8>(MaxModuleRarity)) continue;

		FMAShopStockEntry Entry;
		Entry.StockId = Stock.Num();
		Entry.VisualSeed = FMath::Rand();
		Entry.SkillModule = SkillModule;
		Entry.Price = ResolveModulePrice(SkillModule);
		const FMASkillIconData IconData = SkillModule->ResolveIconData(ModuleQualityData);
		Entry.Icon = IconData.Icon;
		Entry.IconColor = IconData.IconColor;
		Entry.InnerColor = IconData.InnerColor;
		Entry.QualityColor = SkillModule->ResolveFrameColor(ModuleQualityData);
		if (ModuleQualityData)
		{
			const FMAModuleQuality& Quality = SkillModule->GetModuleQuality();
			if (const FMAModuleRarityData* RarityData = ModuleQualityData->FindRarityData(Quality.Rarity))
			{
				Entry.QualityText = RarityData->DisplayName;
				Entry.GlowAlpha = RarityData->GlowAlpha;
			}
		}
		Stock.Add(Entry);
	}
	return Stock;
}

int32 AMAShopNPC::ResolveModulePrice(const UMASkillModule* SkillModule) const
{
	if (!SkillModule) return 0;
	check(ModuleQualityData);
	return ModuleQualityData->ResolvePrice(SkillModule->GetModuleQuality());
}

void AMAShopNPC::OnRep_CurrentStockEntries()
{
	if (ActiveShopWidget) ActiveShopWidget->RefreshStock();
}
