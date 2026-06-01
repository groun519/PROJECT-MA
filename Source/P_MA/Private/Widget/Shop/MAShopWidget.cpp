#include "Widget/Shop/MAShopWidget.h"

#include "AbilitySystemComponent.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "GameplayEffectTypes.h"
#include "GAS/MAPlayerAttributeSet.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/MAPlayerController.h"
#include "Player/Components/MACurrencyComponent.h"
#include "Shop/MAShopNPC.h"
#include "Widget/Shop/MAShopDetailWidget.h"
#include "Widget/Shop/MAShopItemWidget.h"

void UMAShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CloseButton->OnClicked.RemoveDynamic(this, &UMAShopWidget::HandleCloseButtonClicked);
	CloseButton->OnClicked.AddDynamic(this, &UMAShopWidget::HandleCloseButtonClicked);

	DetailWidget->OnBuyRequested.RemoveAll(this);
	DetailWidget->OnBuyRequested.AddUObject(this, &UMAShopWidget::HandleBuyRequested);
	DetailWidget->SetEntry(nullptr);
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
	RebuildItems();
}

void UMAShopWidget::RefreshStock()
{
	RefreshCoinText();
	RebuildItems();
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
		->GetGameplayAttributeValueChangeDelegate(UMAPlayerAttributeSet::GetCoinAttribute())
		.AddUObject(this, &UMAShopWidget::HandleCoinAttributeChanged);
	BoundCoinAbilitySystemComponent = AbilitySystemComponent;
}

void UMAShopWidget::UnbindCoinAttributeChanged()
{
	if (BoundCoinAbilitySystemComponent && CoinAttributeChangedHandle.IsValid())
	{
		BoundCoinAbilitySystemComponent
			->GetGameplayAttributeValueChangeDelegate(UMAPlayerAttributeSet::GetCoinAttribute())
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

void UMAShopWidget::RebuildItems()
{
	ItemContainer->ClearChildren();
	if (!ShopNPC || !ItemWidgetClass)
	{
		PendingSelectionIndex = INDEX_NONE;
		HandleItemSelected(INDEX_NONE);
		return;
	}

	const TArray<FMAShopStockEntry>& StockEntries = ShopNPC->GetCurrentStockEntries();
	for (int32 EntryIndex = 0; EntryIndex < StockEntries.Num(); ++EntryIndex)
	{
		UMAShopItemWidget* ItemWidget = CreateWidget<UMAShopItemWidget>(this, ItemWidgetClass);

		ItemWidget->InitializeItem(StockEntries[EntryIndex]);
		ItemWidget->OnItemSelected.AddUObject(this, &UMAShopWidget::HandleItemSelected);
		ItemContainer->AddChild(ItemWidget);
	}

	if (StockEntries.IsEmpty())
	{
		PendingSelectionIndex = INDEX_NONE;
		HandleItemSelected(INDEX_NONE);
		return;
	}

	const int32 SelectedStockIndex = StockEntries.IndexOfByPredicate([this](const FMAShopStockEntry& Entry)
	{
		return Entry.StockId == SelectedStockId;
	});

	int32 StockIdToSelect = SelectedStockIndex != INDEX_NONE ? SelectedStockId : StockEntries[0].StockId;
	if (SelectedStockIndex == INDEX_NONE && PendingSelectionIndex != INDEX_NONE)
	{
		const int32 SelectionIndex = FMath::Clamp(PendingSelectionIndex, 0, StockEntries.Num() - 1);
		StockIdToSelect = StockEntries[SelectionIndex].StockId;
	}
	PendingSelectionIndex = INDEX_NONE;

	HandleItemSelected(StockIdToSelect);
}

void UMAShopWidget::HandleItemSelected(int32 StockId)
{
	SelectedStockId = StockId;
	const TArray<FMAShopStockEntry>* StockEntries = ShopNPC ? &ShopNPC->GetCurrentStockEntries() : nullptr;
	const FMAShopStockEntry* Entry = StockEntries
		? StockEntries->FindByPredicate([StockId](const FMAShopStockEntry& Candidate)
		{
			return Candidate.StockId == StockId;
		})
		: nullptr;
	DetailWidget->SetEntry(Entry);

	for (int32 ChildIndex = 0; ChildIndex < ItemContainer->GetChildrenCount(); ++ChildIndex)
	{
		if (UMAShopItemWidget* ItemWidget = Cast<UMAShopItemWidget>(ItemContainer->GetChildAt(ChildIndex)))
		{
			ItemWidget->SetSelected(ItemWidget->GetStockId() == SelectedStockId);
		}
	}
}

void UMAShopWidget::HandleBuyRequested()
{
	if (!ShopNPC || SelectedStockId == INDEX_NONE) return;

	const TArray<FMAShopStockEntry>& StockEntries = ShopNPC->GetCurrentStockEntries();
	PendingSelectionIndex = StockEntries.IndexOfByPredicate([this](const FMAShopStockEntry& Entry)
	{
		return Entry.StockId == SelectedStockId;
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
