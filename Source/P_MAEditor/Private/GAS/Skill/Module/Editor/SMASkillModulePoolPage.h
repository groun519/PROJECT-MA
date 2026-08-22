#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "Widgets/SCompoundWidget.h"

class ITableRow;
class STableViewBase;
class UMASkillModulePool;
template<typename ItemType> class SListView;

struct FMASkillModulePoolAssetListItem
{
	FSoftObjectPath AssetPath;
	FName AssetName;
};

using FMASkillModulePoolAssetItem = TSharedPtr<FMASkillModulePoolAssetListItem>;

struct FMASkillModulePoolModuleListItem
{
	int32 ModuleId = 0;
	FName ModuleName;
	EMAModuleRarity Rarity = EMAModuleRarity::Rarity4;
	FText RarityName;
	FLinearColor RarityColor = FLinearColor::White;
	float SelectionShare = 0.f;
	bool bMissing = false;
};

using FMASkillModulePoolModuleItem = TSharedPtr<FMASkillModulePoolModuleListItem>;

class SMASkillModulePoolPage : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMASkillModulePoolPage) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void Refresh(const FString& InSourceDirectory);

private:
	FReply CreatePool();
	FReply DeletePool();
	FReply SavePool();
	FReply RefreshPage();
	FReply AddModule(FMASkillModulePoolModuleItem Item);
	FReply RemoveModule(FMASkillModulePoolModuleItem Item);

	void RefreshPools(const FSoftObjectPath& SelectPath = FSoftObjectPath());
	void RefreshModuleCatalog();
	void RefreshModuleLists();
	void OnPoolSelected(FMASkillModulePoolAssetItem Item, ESelectInfo::Type SelectInfo);
	TSharedRef<ITableRow> GeneratePoolRow(
		FMASkillModulePoolAssetItem Item,
		const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> GenerateSelectedModuleRow(
		FMASkillModulePoolModuleItem Item,
		const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> GenerateAvailableModuleRow(
		FMASkillModulePoolModuleItem Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	FString SourceDirectory;
	FText StatusText;
	TWeakObjectPtr<UMASkillModulePool> SelectedPool;
	TArray<FMASkillModulePoolAssetItem> PoolItems;
	TArray<FMASkillModulePoolModuleItem> ModuleCatalog;
	TArray<FMASkillModulePoolModuleItem> SelectedModuleItems;
	TArray<FMASkillModulePoolModuleItem> AvailableModuleItems;
	TArray<TSharedPtr<int32>> RarityFilterOptions;
	TSharedPtr<SListView<FMASkillModulePoolAssetItem>> PoolListView;
	TSharedPtr<SListView<FMASkillModulePoolModuleItem>> SelectedModuleListView;
	TSharedPtr<SListView<FMASkillModulePoolModuleItem>> AvailableModuleListView;
	FString ModuleSearchText;
	int32 RarityFilter = INDEX_NONE;
};
