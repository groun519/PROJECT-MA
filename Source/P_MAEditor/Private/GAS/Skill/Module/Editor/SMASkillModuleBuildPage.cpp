#include "GAS/Skill/Module/Editor/SMASkillModuleBuildPage.h"

#include "GAS/Skill/Module/Build/MASkillModuleBuildPipeline.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Styling/AppStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "MASkillModuleBuildPage"

static const FName ActionColumn = TEXT("Action");
static const FName ModuleIdColumn = TEXT("ModuleId");
static const FName SourceColumn = TEXT("Source");
static const FName StatusColumn = TEXT("Status");
static const FName LastBuiltColumn = TEXT("LastBuilt");

static FText GetBuildStatusText(const EMASkillModuleBuildStatus Status)
{
	switch (Status)
	{
	case EMASkillModuleBuildStatus::NeedsBuild:
		return LOCTEXT("NeedsBuild", "Needs Build");
	case EMASkillModuleBuildStatus::Built:
		return LOCTEXT("Built", "Built");
	case EMASkillModuleBuildStatus::Error:
	default:
		return LOCTEXT("Error", "Error");
	}
}

static bool SortBuildItems(const FMASkillModuleBuildItem& A, const FMASkillModuleBuildItem& B)
{
	if (A.Status != B.Status) return A.Status < B.Status;
	if (A.ModuleId <= 0 || B.ModuleId <= 0)
	{
		if (A.ModuleId > 0) return true;
		if (B.ModuleId > 0) return false;
	}
	if (A.ModuleId != B.ModuleId) return A.ModuleId < B.ModuleId;
	return A.SourceFile < B.SourceFile;
}

static FSlateColor GetBuildStatusBackgroundColor(const EMASkillModuleBuildStatus Status)
{
	if (Status == EMASkillModuleBuildStatus::Error)
	{
		return FLinearColor(0.28f, 0.07f, 0.05f, 0.7f);
	}
	if (Status == EMASkillModuleBuildStatus::NeedsBuild)
	{
		return FLinearColor(0.25f, 0.18f, 0.04f, 0.7f);
	}
	return FLinearColor(0.07f, 0.17f, 0.09f, 0.7f);
}

class SMASkillModuleBuildRow final : public SMultiColumnTableRow<FMASkillModuleBuildListItem>
{
public:
	SLATE_BEGIN_ARGS(SMASkillModuleBuildRow) {}
		SLATE_ARGUMENT(FMASkillModuleBuildListItem, Item)
		SLATE_EVENT(FOnClicked, OnDeleteGeneratedAsset)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
	{
		Item = InArgs._Item;
		OnDeleteGeneratedAsset = InArgs._OnDeleteGeneratedAsset;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments().Padding(2.f), OwnerTable);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (ColumnName == ActionColumn)
		{
			if (!Item->GeneratedAssetPath.IsValid()) return SNew(SSpacer);

			return SNew(SButton)
				.ButtonColorAndOpacity(FLinearColor(0.45f, 0.04f, 0.03f, 1.f))
				.ContentPadding(FMargin(4.f, 1.f))
				.ToolTipText(LOCTEXT("DeleteGeneratedAsset", "Delete generated asset"))
				.OnClicked(OnDeleteGeneratedAsset)
				[
					SNew(SImage)
					.Image(FAppStyle::GetBrush("Icons.Delete"))
				];
		}
		if (ColumnName == StatusColumn)
		{
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.f)
				.Padding(4.f, 2.f, 8.f, 2.f)
				[
					SNew(SBorder)
						.Padding(FMargin(6.f, 1.f))
						.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderBackgroundColor(GetBuildStatusBackgroundColor(Item->Status))
						.ToolTipText(Item->StatusDetail)
						[
							SNew(STextBlock)
								.Text(GetBuildStatusText(Item->Status))
								.Justification(ETextJustify::Center)
						]
				];
		}

		FText Text;
		FText Tooltip;
		if (ColumnName == ModuleIdColumn)
		{
			Text = Item->ModuleId > 0 ? FText::AsNumber(Item->ModuleId) : FText::FromString(TEXT("-"));
		}
		else if (ColumnName == SourceColumn)
		{
			Text = Item->SourceFile.IsEmpty()
				? FText::FromString(Item->ModuleId > 0
					? FString::Printf(TEXT("M_%d.json"), Item->ModuleId)
					: TEXT("-"))
				: FText::FromString(FPaths::GetCleanFilename(Item->SourceFile));
			Tooltip = FText::FromString(Item->SourceFile.IsEmpty()
				? Item->GeneratedAssetPath.ToString()
				: Item->SourceFile);
		}
		else if (ColumnName == LastBuiltColumn)
		{
			Text = Item->LastBuiltAt > 0
				? FText::AsDateTime(
					FDateTime::FromUnixTimestamp(Item->LastBuiltAt),
					EDateTimeStyle::Short,
					EDateTimeStyle::Short)
				: FText::FromString(TEXT("-"));
		}
		return SNew(STextBlock)
			.Text(Text)
			.ToolTipText(Tooltip);
	}

private:
	FMASkillModuleBuildListItem Item;
	FOnClicked OnDeleteGeneratedAsset;
};

void SMASkillModuleBuildPage::Construct(const FArguments&)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 6.f, 0.f)
			[
				SNew(STextBlock).Text(LOCTEXT("SourceDirectory", "JSON Directory"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text_Lambda([this]
				{
					return SourceDirectory.IsEmpty()
						? LOCTEXT("NoSourceDirectory", "No JSON directory selected.")
						: FText::FromString(SourceDirectory);
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.IsEnabled_Lambda([this] { return !SourceDirectory.IsEmpty(); })
				.OnClicked(this, &SMASkillModuleBuildPage::RefreshStatus)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f, 0.f, 6.f, 6.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("BuildSelected", "Build Selected"))
				.IsEnabled_Lambda([this]
				{
					return BuildListView.IsValid()
						&& BuildListView->GetSelectedItems().ContainsByPredicate(
							[](const FMASkillModuleBuildListItem& Item)
							{
								return Item && Item->CanBuild();
							});
				})
				.OnClicked(this, &SMASkillModuleBuildPage::BuildSelected)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("BuildRequired", "Build Required"))
				.IsEnabled_Lambda([this] { return bHasRequiredBuilds; })
				.OnClicked(this, &SMASkillModuleBuildPage::BuildRequired)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RebuildAll", "Rebuild All"))
				.IsEnabled_Lambda([this] { return bHasBuildSources; })
				.OnClicked(this, &SMASkillModuleBuildPage::RebuildAll)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(6.f, 0.f)
		[
			SNew(SBorder)
			.Padding(2.f)
			[
				SAssignNew(BuildListView, SListView<FMASkillModuleBuildListItem>)
				.ListItemsSource(&BuildItems)
				.SelectionMode(ESelectionMode::Multi)
				.OnGenerateRow(this, &SMASkillModuleBuildPage::GenerateBuildRow)
				.HeaderRow
				(
					SNew(SHeaderRow)
					+ SHeaderRow::Column(ActionColumn)
					.FixedWidth(34.f)
					+ SHeaderRow::Column(StatusColumn)
					.DefaultLabel(LOCTEXT("StatusColumn", "Status"))
					.FixedWidth(110.f)
					+ SHeaderRow::Column(ModuleIdColumn)
					.DefaultLabel(LOCTEXT("ModuleIdColumn", "Module ID"))
					.FixedWidth(90.f)
					+ SHeaderRow::Column(SourceColumn)
					.DefaultLabel(LOCTEXT("SourceColumn", "Source"))
					.FillWidth(0.2f)
					+ SHeaderRow::Column(LastBuiltColumn)
					.DefaultLabel(LOCTEXT("LastBuiltColumn", "Last Built"))
					.FixedWidth(150.f)
				)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(STextBlock).Text_Lambda([this] { return StatusText; })
		]
	];
}

void SMASkillModuleBuildPage::Refresh(const FString& InSourceDirectory)
{
	SourceDirectory = InSourceDirectory;
	RefreshItems();
}

FReply SMASkillModuleBuildPage::RefreshStatus()
{
	RefreshItems();
	return FReply::Handled();
}

FReply SMASkillModuleBuildPage::BuildSelected()
{
	TArray<FString> SourceFiles;
	for (const FMASkillModuleBuildListItem& Item : BuildListView->GetSelectedItems())
	{
		if (Item && Item->CanBuild())
		{
			SourceFiles.Add(Item->SourceFile);
		}
	}
	ExecuteBuild(SourceFiles, EMASkillModuleBuildMode::IfRequired);
	return FReply::Handled();
}

FReply SMASkillModuleBuildPage::BuildRequired()
{
	TArray<FString> SourceFiles;
	for (const FMASkillModuleBuildListItem& Item : BuildItems)
	{
		if (Item && Item->RequiresBuild()) SourceFiles.Add(Item->SourceFile);
	}
	ExecuteBuild(SourceFiles, EMASkillModuleBuildMode::IfRequired);
	return FReply::Handled();
}

FReply SMASkillModuleBuildPage::RebuildAll()
{
	TArray<FString> SourceFiles;
	for (const FMASkillModuleBuildListItem& Item : BuildItems)
	{
		if (Item && Item->CanBuild())
		{
			SourceFiles.Add(Item->SourceFile);
		}
	}
	ExecuteBuild(SourceFiles, EMASkillModuleBuildMode::Force);
	return FReply::Handled();
}

FReply SMASkillModuleBuildPage::DeleteGeneratedAsset(const FMASkillModuleBuildListItem Item)
{
	if (!Item || !Item->GeneratedAssetPath.IsValid()) return FReply::Handled();

	const EAppReturnType::Type Confirmation = FMessageDialog::Open(
		EAppMsgType::YesNo,
		FText::Format(
			LOCTEXT(
				"ConfirmDeleteGeneratedAsset",
				"Delete generated asset '{0}'?\nThe source JSON will not be changed."),
			FText::FromString(Item->GeneratedAssetPath.GetAssetName())));
	if (Confirmation != EAppReturnType::Yes) return FReply::Handled();

	FText Error;
	if (!FMASkillModuleBuildPipeline::DeleteGeneratedAsset(Item->GeneratedAssetPath, Error))
	{
		ShowError(Error);
		return FReply::Handled();
	}

	if (!RefreshItems()) return FReply::Handled();
	StatusText = LOCTEXT("DeleteGeneratedAssetSucceeded", "Deleted the generated module asset.");
	return FReply::Handled();
}

TSharedRef<ITableRow> SMASkillModuleBuildPage::GenerateBuildRow(
	FMASkillModuleBuildListItem Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SMASkillModuleBuildRow, OwnerTable)
		.Item(Item)
		.OnDeleteGeneratedAsset(
			FOnClicked::CreateSP(this, &SMASkillModuleBuildPage::DeleteGeneratedAsset, Item));
}

bool SMASkillModuleBuildPage::RefreshItems()
{
	TArray<FMASkillModuleBuildItem> Items;
	FText Error;
	if (!FMASkillModuleBuildPipeline::CollectStatus(SourceDirectory, Items, Error))
	{
		BuildListView->ClearSelection();
		BuildItems.Reset();
		bHasBuildSources = false;
		bHasRequiredBuilds = false;
		BuildListView->RequestListRefresh();
		ShowError(Error);
		return false;
	}

	BuildListView->ClearSelection();
	Items.Sort(&SortBuildItems);
	BuildItems.Reset(Items.Num());
	bHasBuildSources = false;
	bHasRequiredBuilds = false;
	int32 ErrorCount = 0;
	int32 NeedsBuildCount = 0;
	int32 BuiltCount = 0;
	for (FMASkillModuleBuildItem& Item : Items)
	{
		bHasBuildSources |= Item.CanBuild();
		if (Item.RequiresBuild())
		{
			++NeedsBuildCount;
			bHasRequiredBuilds = true;
		}
		else if (Item.Status == EMASkillModuleBuildStatus::Built) ++BuiltCount;
		else ++ErrorCount;
		BuildItems.Add(MakeShared<FMASkillModuleBuildItem>(MoveTemp(Item)));
	}
	BuildListView->RequestListRefresh();
	StatusText = FText::Format(
		LOCTEXT("StatusSummary", "Total {0}, error {1}, needs build {2}, built {3}."),
		BuildItems.Num(),
		ErrorCount,
		NeedsBuildCount,
		BuiltCount);
	return true;
}

void SMASkillModuleBuildPage::ExecuteBuild(
	const TArray<FString>& SourceFiles,
	const EMASkillModuleBuildMode BuildMode)
{
	if (SourceFiles.IsEmpty()) return;

	FMASkillModuleBuildSummary Summary;
	FText Error;
	const bool bSucceeded = FMASkillModuleBuildPipeline::BuildFiles(
		SourceDirectory,
		SourceFiles,
		BuildMode,
		Summary,
		Error);
	if (!RefreshItems()) return;
	if (!bSucceeded)
	{
		ShowError(Error);
		return;
	}

	StatusText = FText::Format(
		LOCTEXT("BuildSucceeded", "Built {0}, up to date {1}."),
		Summary.Built,
		Summary.UpToDate);
}

void SMASkillModuleBuildPage::ShowError(const FText& Error)
{
	StatusText = Error;
	FMessageDialog::Open(EAppMsgType::Ok, Error);
}

#undef LOCTEXT_NAMESPACE
