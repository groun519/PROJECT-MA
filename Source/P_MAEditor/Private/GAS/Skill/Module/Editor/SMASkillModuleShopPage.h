#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "Widgets/SCompoundWidget.h"

class ITableRow;
class STableViewBase;
class UMAShopModulePool;
template<typename ItemType> class SListView;

struct FMASkillModuleShopAssetListItem
{
	FSoftObjectPath AssetPath;
	FName AssetName;
};

using FMASkillModuleShopAssetItem = TSharedPtr<FMASkillModuleShopAssetListItem>;

struct FMASkillModuleShopModuleListItem
{
	int32 ModuleId = 0;
	FName ModuleName;
	EMAModuleRarity Rarity = EMAModuleRarity::Rarity4;
	FText RarityName;
	FLinearColor RarityColor = FLinearColor::White;
	float SelectionShare = 0.f;
	bool bMissing = false;
};

using FMASkillModuleShopModuleItem = TSharedPtr<FMASkillModuleShopModuleListItem>;

class SMASkillModuleShopPage : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMASkillModuleShopPage) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void Refresh(const FString& InSourceDirectory);

private:
	FReply CreateShop();
	FReply DeleteShop();
	FReply SaveShop();
	FReply RefreshPage();
	FReply AddModule(FMASkillModuleShopModuleItem Item);
	FReply RemoveModule(FMASkillModuleShopModuleItem Item);

	void RefreshShops(const FSoftObjectPath& SelectPath = FSoftObjectPath());
	void RefreshModuleCatalog();
	void RefreshModuleLists();
	void OnShopSelected(FMASkillModuleShopAssetItem Item, ESelectInfo::Type SelectInfo);
	TSharedRef<ITableRow> GenerateShopRow(
		FMASkillModuleShopAssetItem Item,
		const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> GenerateSelectedModuleRow(
		FMASkillModuleShopModuleItem Item,
		const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> GenerateAvailableModuleRow(
		FMASkillModuleShopModuleItem Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	FString SourceDirectory;
	FText StatusText;
	TWeakObjectPtr<UMAShopModulePool> SelectedShop;
	TArray<FMASkillModuleShopAssetItem> ShopItems;
	TArray<FMASkillModuleShopModuleItem> ModuleCatalog;
	TArray<FMASkillModuleShopModuleItem> SelectedModuleItems;
	TArray<FMASkillModuleShopModuleItem> AvailableModuleItems;
	TArray<TSharedPtr<int32>> RarityFilterOptions;
	TSharedPtr<SListView<FMASkillModuleShopAssetItem>> ShopListView;
	TSharedPtr<SListView<FMASkillModuleShopModuleItem>> SelectedModuleListView;
	TSharedPtr<SListView<FMASkillModuleShopModuleItem>> AvailableModuleListView;
	FString ModuleSearchText;
	int32 RarityFilter = INDEX_NONE;
};
