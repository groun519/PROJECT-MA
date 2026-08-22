#include "GAS/Skill/Module/Editor/SMASkillModulePoolPage.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Editor.h"
#include "Factories/DataAssetFactory.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonFile.h"
#include "HAL/FileManager.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "ObjectTools.h"
#include "Setting/MAGameSettings.h"
#include "GAS/Skill/Module/MASkillModulePool.h"
#include "Styling/AppStyle.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "MASkillModulePoolPage"

static FText ResolveModuleLabel(const FMASkillModulePoolModuleListItem& Item)
{
	const FString ModuleName = Item.ModuleName.IsNone()
		? (Item.bMissing ? TEXT("Missing Module") : TEXT("Unnamed Module"))
		: Item.ModuleName.ToString();
	return FText::FromString(FString::Printf(TEXT("#%d  %s"), Item.ModuleId, *ModuleName));
}

static FText ResolveRarityLabel(const int32 RarityIndex)
{
	if (RarityIndex == INDEX_NONE) return LOCTEXT("AllRarities", "All Rarities");

	const EMAModuleRarity Rarity = static_cast<EMAModuleRarity>(RarityIndex);
	const UMAModuleQualityData* QualityData = UMAGameSettings::Get()->GetModuleQualityData();
	if (QualityData)
	{
		if (const FMAModuleRarityData* RarityData = QualityData->FindRarityData(Rarity))
		{
			if (!RarityData->DisplayName.IsEmpty()) return RarityData->DisplayName;
		}
	}

	return FText::Format(
		LOCTEXT("RarityFallback", "Rarity {0}"),
		FText::AsNumber(RarityIndex + 1));
}

void SMASkillModulePoolPage::Construct(const FArguments&)
{
	RarityFilterOptions.Add(MakeShared<int32>(INDEX_NONE));
	for (int32 Index = 0; Index <= static_cast<int32>(EMAModuleRarity::Rarity7); ++Index)
	{
		RarityFilterOptions.Add(MakeShared<int32>(Index));
	}

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(6.f)
		[
			SNew(SSplitter)
			.PhysicalSplitterHandleSize(2.f)
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				SNew(SBorder)
				.Padding(6.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Pools", "Pools"))
						.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.Text(LOCTEXT("NewPool", "New"))
							.OnClicked(this, &SMASkillModulePoolPage::CreatePool)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(4.f, 0.f, 0.f, 0.f)
						[
							SNew(SButton)
							.Text(LOCTEXT("DeletePool", "Delete"))
							.IsEnabled_Lambda([this] { return SelectedPool.IsValid(); })
							.OnClicked(this, &SMASkillModulePoolPage::DeletePool)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(4.f, 0.f, 0.f, 0.f)
						[
							SNew(SButton)
							.Text(LOCTEXT("Refresh", "Refresh"))
							.OnClicked(this, &SMASkillModulePoolPage::RefreshPage)
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SAssignNew(PoolListView, SListView<FMASkillModulePoolAssetItem>)
						.ListItemsSource(&PoolItems)
						.SelectionMode(ESelectionMode::Single)
						.OnGenerateRow(this, &SMASkillModulePoolPage::GeneratePoolRow)
						.OnSelectionChanged(this, &SMASkillModulePoolPage::OnPoolSelected)
					]
				]
			]
			+ SSplitter::Slot()
			.Value(0.375f)
			[
				SNew(SBorder)
				.Padding(6.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text_Lambda([this]
							{
								return SelectedPool.IsValid()
									? FText::FromString(SelectedPool->GetName())
									: LOCTEXT("NoPoolSelected", "Pool Modules");
							})
							.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(SButton)
							.Text(LOCTEXT("SavePool", "Save"))
							.IsEnabled_Lambda([this] { return SelectedPool.IsValid(); })
							.OnClicked(this, &SMASkillModulePoolPage::SavePool)
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SAssignNew(SelectedModuleListView, SListView<FMASkillModulePoolModuleItem>)
						.ListItemsSource(&SelectedModuleItems)
						.SelectionMode(ESelectionMode::None)
						.OnGenerateRow(this, &SMASkillModulePoolPage::GenerateSelectedModuleRow)
					]
				]
			]
			+ SSplitter::Slot()
			.Value(0.375f)
			[
				SNew(SBorder)
				.Padding(6.f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("AvailableModules", "Available Modules"))
						.Font(FAppStyle::GetFontStyle("DetailsView.CategoryFontStyle"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.f, 0.f, 0.f, 6.f)
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.FillWidth(1.f)
						[
							SNew(SSearchBox)
							.HintText(LOCTEXT("SearchModules", "Search name or ID"))
							.OnTextChanged_Lambda([this](const FText& SearchText)
							{
								ModuleSearchText = SearchText.ToString().TrimStartAndEnd();
								RefreshModuleLists();
							})
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(6.f, 0.f, 0.f, 0.f)
						[
							SNew(SBox)
							.WidthOverride(120.f)
							[
								SNew(SComboBox<TSharedPtr<int32>>)
								.OptionsSource(&RarityFilterOptions)
								.InitiallySelectedItem(RarityFilterOptions[0])
								.OnGenerateWidget_Lambda([](const TSharedPtr<int32> Item)
								{
									return SNew(STextBlock).Text(ResolveRarityLabel(Item ? *Item : INDEX_NONE));
								})
								.OnSelectionChanged_Lambda([this](const TSharedPtr<int32> Item, ESelectInfo::Type)
								{
									RarityFilter = Item ? *Item : INDEX_NONE;
									RefreshModuleLists();
								})
								[
									SNew(STextBlock).Text_Lambda([this]
									{
										return ResolveRarityLabel(RarityFilter);
									})
								]
							]
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SAssignNew(AvailableModuleListView, SListView<FMASkillModulePoolModuleItem>)
						.ListItemsSource(&AvailableModuleItems)
						.SelectionMode(ESelectionMode::None)
						.OnGenerateRow(this, &SMASkillModulePoolPage::GenerateAvailableModuleRow)
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f, 0.f, 6.f, 6.f)
		[
			SNew(STextBlock).Text_Lambda([this] { return StatusText; })
		]
	];
}

void SMASkillModulePoolPage::Refresh(const FString& InSourceDirectory)
{
	SourceDirectory = InSourceDirectory;
	RefreshModuleCatalog();
	RefreshPools();
}

FReply SMASkillModulePoolPage::CreatePool()
{
	UDataAssetFactory* Factory = NewObject<UDataAssetFactory>();
	Factory->DataAssetClass = UMASkillModulePool::StaticClass();
	UMASkillModulePool* NewPool = Cast<UMASkillModulePool>(
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"))
		.Get()
		.CreateAssetWithDialog(
			TEXT("NewPool"),
			TEXT("/Game"),
			UMASkillModulePool::StaticClass(),
			Factory,
			NAME_None,
			false));
	if (!NewPool) return FReply::Handled();

	StatusText = LOCTEXT("PoolCreated", "Created a skill module pool.");
	RefreshPools(FSoftObjectPath(NewPool));
	return FReply::Handled();
}

FReply SMASkillModulePoolPage::DeletePool()
{
	UMASkillModulePool* Pool = SelectedPool.Get();
	if (!Pool) return FReply::Handled();

	FAssetRegistryModule& AssetRegistry =
		FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	const FAssetData AssetData = AssetRegistry.Get().GetAssetByObjectPath(FSoftObjectPath(Pool));
	if (AssetData.IsValid() && ObjectTools::DeleteAssets({AssetData}, true) == 1)
	{
		SelectedPool.Reset();
		StatusText = LOCTEXT("PoolDeleted", "Deleted the skill module pool.");
		RefreshPools();
	}
	return FReply::Handled();
}

FReply SMASkillModulePoolPage::SavePool()
{
	UMASkillModulePool* Pool = SelectedPool.Get();
	UEditorAssetSubsystem* AssetSubsystem = GEditor
		? GEditor->GetEditorSubsystem<UEditorAssetSubsystem>()
		: nullptr;
	if (!Pool || !AssetSubsystem || !AssetSubsystem->SaveLoadedAsset(Pool))
	{
		StatusText = LOCTEXT("SavePoolFailed", "Failed to save the selected skill module pool.");
		FMessageDialog::Open(EAppMsgType::Ok, StatusText, LOCTEXT("ErrorTitle", "Skill Module Pool Error"));
		return FReply::Handled();
	}

	StatusText = LOCTEXT("PoolSaved", "Saved the skill module pool.");
	return FReply::Handled();
}

FReply SMASkillModulePoolPage::RefreshPage()
{
	RefreshModuleCatalog();
	RefreshPools(SelectedPool.IsValid() ? FSoftObjectPath(SelectedPool.Get()) : FSoftObjectPath());
	return FReply::Handled();
}

FReply SMASkillModulePoolPage::AddModule(const FMASkillModulePoolModuleItem Item)
{
	UMASkillModulePool* Pool = SelectedPool.Get();
	if (!Pool || !Item) return FReply::Handled();

	Pool->Modify();
	if (Pool->AddModuleId(Item->ModuleId))
	{
		Pool->MarkPackageDirty();
		RefreshModuleLists();
	}
	return FReply::Handled();
}

FReply SMASkillModulePoolPage::RemoveModule(const FMASkillModulePoolModuleItem Item)
{
	UMASkillModulePool* Pool = SelectedPool.Get();
	if (!Pool || !Item) return FReply::Handled();

	Pool->Modify();
	if (Pool->RemoveModuleId(Item->ModuleId))
	{
		Pool->MarkPackageDirty();
		RefreshModuleLists();
	}
	return FReply::Handled();
}

void SMASkillModulePoolPage::RefreshPools(const FSoftObjectPath& SelectPath)
{
	const FSoftObjectPath PreviousPath = SelectPath.IsValid()
		? SelectPath
		: (SelectedPool.IsValid() ? FSoftObjectPath(SelectedPool.Get()) : FSoftObjectPath());
	PoolItems.Reset();

	FARFilter Filter;
	Filter.ClassPaths.Add(UMASkillModulePool::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;
	TArray<FAssetData> Assets;
	FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"))
		.Get()
		.GetAssets(Filter, Assets);
	Assets.Sort([](const FAssetData& A, const FAssetData& B)
	{
		return A.AssetName.LexicalLess(B.AssetName);
	});

	FMASkillModulePoolAssetItem ItemToSelect;
	for (const FAssetData& Asset : Assets)
	{
		FMASkillModulePoolAssetItem Item = MakeShared<FMASkillModulePoolAssetListItem>();
		Item->AssetPath = Asset.GetSoftObjectPath();
		Item->AssetName = Asset.AssetName;
		if (Item->AssetPath == PreviousPath) ItemToSelect = Item;
		PoolItems.Add(MoveTemp(Item));
	}

	PoolListView->RequestListRefresh();
	if (ItemToSelect)
	{
		PoolListView->SetSelection(ItemToSelect);
	}
	else
	{
		PoolListView->ClearSelection();
		SelectedPool.Reset();
		RefreshModuleLists();
	}
}

void SMASkillModulePoolPage::RefreshModuleCatalog()
{
	ModuleCatalog.Reset();
	if (SourceDirectory.IsEmpty()) return;
	const UMAModuleQualityData* QualityData = UMAGameSettings::Get()->GetModuleQualityData();

	TArray<FString> JsonFiles;
	IFileManager::Get().FindFilesRecursive(JsonFiles, *SourceDirectory, TEXT("*.json"), true, false);
	for (const FString& JsonFile : JsonFiles)
	{
		FMASkillModuleJsonHeader Header;
		FText Error;
		if (!FMASkillModuleJsonFile::ReadHeader(JsonFile, Header, Error)
			|| !FMASkillModuleJsonFile::ValidateSourceFilePath(
				SourceDirectory,
				JsonFile,
				Header.ModuleType,
				Error))
		{
			continue;
		}

		FMASkillModulePoolModuleItem Item = MakeShared<FMASkillModulePoolModuleListItem>();
		Item->ModuleId = Header.ModuleId;
		Item->ModuleName = Header.ModuleName;
		Item->Rarity = Header.ModuleRarity;
		Item->RarityName = ResolveRarityLabel(static_cast<int32>(Header.ModuleRarity));
		if (QualityData)
		{
			if (const FMAModuleRarityData* RarityData = QualityData->FindRarityData(Header.ModuleRarity))
			{
				Item->RarityColor = RarityData->Color;
			}
		}
		ModuleCatalog.Add(MoveTemp(Item));
	}
	ModuleCatalog.Sort([](
		const FMASkillModulePoolModuleItem& A,
		const FMASkillModulePoolModuleItem& B)
	{
		return A->ModuleId < B->ModuleId;
	});
}

void SMASkillModulePoolPage::RefreshModuleLists()
{
	SelectedModuleItems.Reset();
	AvailableModuleItems.Reset();

	UMASkillModulePool* Pool = SelectedPool.Get();
	TSet<int32> SelectedModuleIds;
	if (Pool)
	{
		TMap<int32, FMASkillModulePoolModuleItem> ModulesById;
		for (const FMASkillModulePoolModuleItem& Item : ModuleCatalog)
		{
			ModulesById.FindOrAdd(Item->ModuleId, Item);
		}

		for (const int32 ModuleId : Pool->GetModuleIds())
		{
			SelectedModuleIds.Add(ModuleId);
			if (const FMASkillModulePoolModuleItem* Item = ModulesById.Find(ModuleId))
			{
				SelectedModuleItems.Add(*Item);
				continue;
			}

			FMASkillModulePoolModuleItem MissingItem = MakeShared<FMASkillModulePoolModuleListItem>();
			MissingItem->ModuleId = ModuleId;
			MissingItem->bMissing = true;
			SelectedModuleItems.Add(MoveTemp(MissingItem));
		}
	}
	int32 ResolvedModuleCount = 0;
	for (const FMASkillModulePoolModuleItem& Item : SelectedModuleItems)
	{
		if (Item && !Item->bMissing) ++ResolvedModuleCount;
	}
	if (ResolvedModuleCount > 0)
	{
		const float SelectionShare = 1.f / ResolvedModuleCount;
		for (const FMASkillModulePoolModuleItem& Item : SelectedModuleItems)
		{
			if (Item && !Item->bMissing) Item->SelectionShare = SelectionShare;
		}
	}

	for (const FMASkillModulePoolModuleItem& Item : ModuleCatalog)
	{
		if (SelectedModuleIds.Contains(Item->ModuleId)) continue;
		if (RarityFilter != INDEX_NONE && static_cast<int32>(Item->Rarity) != RarityFilter) continue;
		if (!ModuleSearchText.IsEmpty()
			&& !Item->ModuleName.ToString().Contains(ModuleSearchText, ESearchCase::IgnoreCase)
			&& !LexToString(Item->ModuleId).Contains(ModuleSearchText, ESearchCase::IgnoreCase)) continue;

		AvailableModuleItems.Add(Item);
	}

	SelectedModuleListView->RequestListRefresh();
	AvailableModuleListView->RequestListRefresh();
}

void SMASkillModulePoolPage::OnPoolSelected(
	const FMASkillModulePoolAssetItem Item,
	ESelectInfo::Type)
{
	SelectedPool = Item ? Cast<UMASkillModulePool>(Item->AssetPath.TryLoad()) : nullptr;
	RefreshModuleLists();
}

TSharedRef<ITableRow> SMASkillModulePoolPage::GeneratePoolRow(
	const FMASkillModulePoolAssetItem Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FMASkillModulePoolAssetItem>, OwnerTable)
		.Padding(FMargin(4.f, 2.f))
		.ToolTipText(Item ? FText::FromString(Item->AssetPath.ToString()) : FText::GetEmpty())
		[
			SNew(STextBlock).Text(Item ? FText::FromName(Item->AssetName) : FText::GetEmpty())
		];
}

TSharedRef<ITableRow> SMASkillModulePoolPage::GenerateSelectedModuleRow(
	const FMASkillModulePoolModuleItem Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FMASkillModulePoolModuleItem>, OwnerTable)
		.Padding(FMargin(4.f, 2.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Item ? ResolveModuleLabel(*Item) : FText::GetEmpty())
				.ColorAndOpacity(Item && Item->bMissing
					? FSlateColor(FLinearColor(0.9f, 0.2f, 0.15f))
					: FSlateColor::UseForeground())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(90.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SProgressBar)
						.Percent(Item && !Item->bMissing ? Item->SelectionShare : 0.f)
						.FillColorAndOpacity(Item ? Item->RarityColor : FLinearColor::White)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Item && !Item->bMissing
							? FText::AsPercent(Item->SelectionShare)
							: FText::FromString(TEXT("-")))
					]
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RemoveModule", "Remove"))
				.OnClicked(this, &SMASkillModulePoolPage::RemoveModule, Item)
			]
		];
}

TSharedRef<ITableRow> SMASkillModulePoolPage::GenerateAvailableModuleRow(
	const FMASkillModulePoolModuleItem Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<FMASkillModulePoolModuleItem>, OwnerTable)
		.Padding(FMargin(4.f, 2.f))
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 6.f, 0.f)
			[
				SNew(SColorBlock)
				.Color(Item ? Item->RarityColor : FLinearColor::White)
				.Size(FVector2D(8.f, 16.f))
				.ToolTipText(Item ? Item->RarityName : FText::GetEmpty())
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock).Text(Item ? ResolveModuleLabel(*Item) : FText::GetEmpty())
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.f, 0.f)
			[
				SNew(STextBlock)
				.Text(Item ? Item->RarityName : FText::GetEmpty())
				.ColorAndOpacity(Item ? Item->RarityColor : FLinearColor::White)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("AddModule", "Add"))
				.IsEnabled_Lambda([this] { return SelectedPool.IsValid(); })
				.OnClicked(this, &SMASkillModulePoolPage::AddModule, Item)
			]
		];
}

#undef LOCTEXT_NAMESPACE
