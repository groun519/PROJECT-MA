#include "Widget/Shop/MAShopWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "GameplayEffectTypes.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerController.h"
#include "Player/Components/MACurrencyComponent.h"
#include "Shop/MAShopNPC.h"
#include "Widget/Shop/MAShopDetailWidget.h"
#include "Widget/Shop/MAShopProductWidget.h"

void UMAShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CloseButton->OnClicked.RemoveDynamic(this, &UMAShopWidget::HandleCloseButtonClicked);
	CloseButton->OnClicked.AddDynamic(this, &UMAShopWidget::HandleCloseButtonClicked);

	DetailWidget->OnBuyRequested.RemoveAll(this);
	DetailWidget->OnBuyRequested.AddUObject(this, &UMAShopWidget::HandleBuyRequested);
	DetailWidget->SetProduct(nullptr);
}

void UMAShopWidget::NativeDestruct()
{
	UnbindCoinAttributeChanged();
	Super::NativeDestruct();
}

void UMAShopWidget::InitializeShop(AMAShopNPC* InShopNPC)
{
	ShopNPC = InShopNPC;
	BindCoinAttributeChanged();
	RefreshCoinText();
	RebuildProducts();
}

void UMAShopWidget::RefreshStock()
{
	RefreshCoinText();
	RebuildProducts();
}

void UMAShopWidget::HandleCloseButtonClicked()
{
	if (ShopNPC)
	{
		ShopNPC->CloseShop(GetOwningPlayer());
	}
}

void UMAShopWidget::BindCoinAttributeChanged()
{
	UnbindCoinAttributeChanged();

	const AMAPlayerCharacter* PlayerCharacter = GetOwningPlayerPawn<AMAPlayerCharacter>();
	UAbilitySystemComponent* AbilitySystemComponent = PlayerCharacter ? PlayerCharacter->GetAbilitySystemComponent() : nullptr;
	if (!AbilitySystemComponent) return;

	CoinAttributeChangedHandle = AbilitySystemComponent
		->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetCoinAttribute())
		.AddUObject(this, &UMAShopWidget::HandleCoinAttributeChanged);
	BoundCoinAbilitySystemComponent = AbilitySystemComponent;
}

void UMAShopWidget::UnbindCoinAttributeChanged()
{
	if (BoundCoinAbilitySystemComponent && CoinAttributeChangedHandle.IsValid())
	{
		BoundCoinAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetCoinAttribute())
			.Remove(CoinAttributeChangedHandle);
	}

	CoinAttributeChangedHandle.Reset();
	BoundCoinAbilitySystemComponent = nullptr;
}

void UMAShopWidget::RefreshCoinText()
{
	if (!CoinText) return;

	const AMAPlayerCharacter* PlayerCharacter = GetOwningPlayerPawn<AMAPlayerCharacter>();
	const UMACurrencyComponent* CurrencyComponent = PlayerCharacter ? PlayerCharacter->GetCurrencyComponent() : nullptr;
	const int32 Coin = CurrencyComponent ? FMath::FloorToInt(CurrencyComponent->GetCoin()) : 0;
	CoinText->SetText(FText::AsNumber(Coin));
}

void UMAShopWidget::RebuildProducts()
{
	ModuleContainer->ClearChildren();
	ItemContainer->ClearChildren();
	if (!ShopNPC || !ProductWidgetClass)
	{
		PendingSelectionIndex = INDEX_NONE;
		HandleProductSelected(INDEX_NONE);
		return;
	}

	const TArray<FMAShopProduct>& Products = ShopNPC->GetCurrentProducts();
	for (const FMAShopProduct& Product : Products)
	{
		UPanelWidget* ProductContainer = nullptr;
		switch (Product.Module->GetModuleType())
		{
		case EMASkillModuleType::Module:
			ProductContainer = ModuleContainer;
			break;
		case EMASkillModuleType::Item:
			ProductContainer = ItemContainer;
			break;
		default:
			continue;
		}

		UMAShopProductWidget* ProductWidget = CreateWidget<UMAShopProductWidget>(this, ProductWidgetClass);
		ProductWidget->SetProduct(Product);
		ProductWidget->OnProductSelected.AddUObject(this, &UMAShopWidget::HandleProductSelected);
		ProductContainer->AddChild(ProductWidget);
	}
	ItemContainer->SetVisibility(ItemContainer->GetChildrenCount() > 0
		? ESlateVisibility::SelfHitTestInvisible
		: ESlateVisibility::Collapsed);

	if (Products.IsEmpty())
	{
		PendingSelectionIndex = INDEX_NONE;
		HandleProductSelected(INDEX_NONE);
		return;
	}

	const int32 SelectedStockIndex = Products.IndexOfByPredicate([this](const FMAShopProduct& Product)
	{
		return Product.StockId == SelectedStockId;
	});

	int32 StockIdToSelect = SelectedStockIndex != INDEX_NONE ? SelectedStockId : Products[0].StockId;
	if (SelectedStockIndex == INDEX_NONE && PendingSelectionIndex != INDEX_NONE)
	{
		const int32 SelectionIndex = FMath::Clamp(PendingSelectionIndex, 0, Products.Num() - 1);
		StockIdToSelect = Products[SelectionIndex].StockId;
	}
	PendingSelectionIndex = INDEX_NONE;

	HandleProductSelected(StockIdToSelect);
}

void UMAShopWidget::HandleProductSelected(int32 StockId)
{
	SelectedStockId = StockId;
	const TArray<FMAShopProduct>* Products = ShopNPC ? &ShopNPC->GetCurrentProducts() : nullptr;
	const FMAShopProduct* Product = Products
		? Products->FindByPredicate([StockId](const FMAShopProduct& Candidate)
		{
			return Candidate.StockId == StockId;
		})
		: nullptr;
	DetailWidget->SetProduct(Product);

	for (UPanelWidget* Container : {ModuleContainer.Get(), ItemContainer.Get()})
	{
		for (int32 ChildIndex = 0; ChildIndex < Container->GetChildrenCount(); ++ChildIndex)
		{
			if (UMAShopProductWidget* ProductWidget = Cast<UMAShopProductWidget>(Container->GetChildAt(ChildIndex)))
			{
				ProductWidget->SetSelected(ProductWidget->GetStockId() == SelectedStockId);
			}
		}
	}
}

void UMAShopWidget::HandleBuyRequested()
{
	if (!ShopNPC || SelectedStockId == INDEX_NONE) return;

	const TArray<FMAShopProduct>& Products = ShopNPC->GetCurrentProducts();
	PendingSelectionIndex = Products.IndexOfByPredicate([this](const FMAShopProduct& Product)
	{
		return Product.StockId == SelectedStockId;
	});

	if (AMAPlayerController* PlayerController = Cast<AMAPlayerController>(GetOwningPlayer()))
	{
		PlayerController->RequestShopPurchase(ShopNPC, SelectedStockId);
	}
}

void UMAShopWidget::HandleCoinAttributeChanged(const FOnAttributeChangeData&)
{
	RefreshCoinText();
}
