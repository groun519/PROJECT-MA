#include "GAS/Skill/Module/Editor/SMASkillModuleEditPage.h"

#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "GAS/Skill/Module/Build/MASkillModuleBuildPipeline.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonFile.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonReader.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonWriter.h"
#include "HAL/FileManager.h"
#include "IDetailsView.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "PropertyEditorModule.h"
#include "Setting/MAGameSettings.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"

#define LOCTEXT_NAMESPACE "MASkillModuleEditPage"

void SMASkillModuleEditPage::Construct(const FArguments&)
{
	EditorObject.Reset(NewObject<UMASkillModuleEditorObject>());
	EditorObject->SetFlags(RF_Transactional);
	FString GeneratedAssetDirectory;
	FText GeneratedAssetDirectoryError;
	FMASkillModuleBuildPipeline::ResolveGeneratedAssetDirectory(
		GeneratedAssetDirectory,
		GeneratedAssetDirectoryError);

	FDetailsViewArgs DetailsArgs;
	DetailsArgs.bAllowSearch = true;
	DetailsArgs.bHideSelectionTip = true;
	DetailsArgs.bLockable = false;
	DetailsArgs.bUpdatesFromSelection = false;
	DetailsArgs.NotifyHook = this;
	DetailsView = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"))
		.CreateDetailView(DetailsArgs);

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
			[
				SAssignNew(SourceDirectoryTextBox, SEditableTextBox)
				.Text(FText::FromString(UMAGameSettings::Get()->SkillModuleJsonDirectory.Path))
				.OnTextCommitted(this, &SMASkillModuleEditPage::OnSourceDirectoryCommitted)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Browse", "Browse"))
				.OnClicked(this, &SMASkillModuleEditPage::BrowseSourceDirectory)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.OnClicked(this, &SMASkillModuleEditPage::RefreshJsonFiles)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f, 0.f, 6.f, 6.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 6.f, 0.f)
			[
				SNew(STextBlock).Text(LOCTEXT("GeneratedAssetDirectory", "Generated Assets"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			[
				SNew(SEditableTextBox)
				.Text(GeneratedAssetDirectoryError.IsEmpty()
					? FText::FromString(GeneratedAssetDirectory)
					: GeneratedAssetDirectoryError)
				.IsReadOnly(true)
				.ToolTipText(LOCTEXT(
					"GeneratedAssetDirectoryTooltip",
					"Configured for the SkillModule type in Project Settings > Game > Asset Manager."))
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(6.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("New", "New"))
				.OnClicked(this, &SMASkillModuleEditPage::NewJson)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Save", "Save"))
				.OnClicked(this, &SMASkillModuleEditPage::SaveCurrentJson)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Build", "Build"))
				.OnClicked(this, &SMASkillModuleEditPage::BuildCurrentJson)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SaveAndBuild", "Save & Build"))
				.OnClicked(this, &SMASkillModuleEditPage::SaveAndBuild)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(6.f, 0.f)
		[
			SNew(SSplitter)
			+ SSplitter::Slot()
			.Value(0.25f)
			[
				SNew(SBorder)
				.Padding(2.f)
				[
					SAssignNew(JsonListView, SListView<FMASkillModuleJsonItem>)
					.ListItemsSource(&JsonItems)
					.OnGenerateRow(this, &SMASkillModuleEditPage::GenerateJsonRow)
					.OnSelectionChanged(this, &SMASkillModuleEditPage::OnJsonSelected)
				]
			]
			+ SSplitter::Slot()
			.Value(0.75f)
			[
				DetailsView.ToSharedRef()
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(STextBlock).Text_Lambda([this] { return StatusText; })
		]
	];

	RefreshJsonFiles();
}

void SMASkillModuleEditPage::NotifyPostChange(
	const FPropertyChangedEvent&,
	FProperty*)
{
	if (bNewJson || !SelectedJsonPath.IsEmpty()) bDirty = true;
}

FReply SMASkillModuleEditPage::BrowseSourceDirectory()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform) return FReply::Handled();

	FString SelectedDirectory;
	const void* ParentWindow = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	if (!DesktopPlatform->OpenDirectoryDialog(
		ParentWindow,
		LOCTEXT("SelectSourceDirectory", "Select Skill Module JSON Directory").ToString(),
		SourceDirectory,
		SelectedDirectory))
	{
		return FReply::Handled();
	}

	if (!ResolvePendingChanges()) return FReply::Handled();
	SourceDirectoryTextBox->SetText(FText::FromString(SelectedDirectory));
	return RefreshJsonFiles();
}

FReply SMASkillModuleEditPage::RefreshJsonFiles()
{
	if (!ResolvePendingChanges())
	{
		SourceDirectoryTextBox->SetText(FText::FromString(SourceDirectory));
		return FReply::Handled();
	}

	if (!CommitSourceDirectory()) return FReply::Handled();
	JsonItems.Reset();
	SelectedJsonItem.Reset();
	SelectedJsonPath.Reset();
	bDirty = false;
	bNewJson = false;
	DetailsView->SetObject(nullptr);

	TArray<FString> JsonFiles;
	IFileManager::Get().FindFilesRecursive(JsonFiles, *SourceDirectory, TEXT("*.json"), true, false);
	JsonItems.Reserve(JsonFiles.Num());
	for (FString& JsonFile : JsonFiles)
	{
		FMASkillModuleJsonItem Item = MakeShared<FMASkillModuleJsonListItem>();
		Item->FilePath = MoveTemp(JsonFile);

		FText IgnoredError;
		EMAModuleRarity IgnoredModuleRarity;
		if (FMASkillModuleJsonFile::ReadHeader(
			Item->FilePath,
			Item->ModuleId,
			Item->ModuleName,
			IgnoredModuleRarity,
			IgnoredError))
		{
			Item->bHeaderValid = true;
		}
		else
		{
			FMASkillModuleJsonFile::ResolveModuleId(
				Item->FilePath,
				Item->ModuleId,
				IgnoredError);
		}
		JsonItems.Add(MoveTemp(Item));
	}
	SortJsonItems();
	JsonListView->RequestListRefresh();
	StatusText = FText::Format(LOCTEXT("JsonCount", "Found {0} module JSON files."), JsonItems.Num());
	return FReply::Handled();
}

void SMASkillModuleEditPage::SortJsonItems()
{
	// Keep ModuleId ascending until selectable sort modes are added.
	JsonItems.Sort([](const FMASkillModuleJsonItem& A, const FMASkillModuleJsonItem& B)
	{
		const bool bAHasModuleId = A->ModuleId > 0;
		const bool bBHasModuleId = B->ModuleId > 0;
		if (bAHasModuleId != bBHasModuleId) return bAHasModuleId;
		if (A->ModuleId != B->ModuleId) return A->ModuleId < B->ModuleId;
		return A->FilePath < B->FilePath;
	});
}

FReply SMASkillModuleEditPage::NewJson()
{
	if (!ResolvePendingChanges()) return FReply::Handled();
	if (!CommitSourceDirectory()) return FReply::Handled();

	EditorObject->SetModule(0, FMASkillModuleData());
	SelectedJsonItem.Reset();
	SelectedJsonPath.Reset();
	bNewJson = true;
	bDirty = true;
	bRestoringSelection = true;
	JsonListView->ClearSelection();
	bRestoringSelection = false;
	DetailsView->SetObject(EditorObject.Get(), true);
	StatusText = LOCTEXT("EditingNewModule", "Editing a new module. ModuleId will be assigned on save.");
	return FReply::Handled();
}

FReply SMASkillModuleEditPage::SaveCurrentJson()
{
	SaveJson();
	return FReply::Handled();
}

FReply SMASkillModuleEditPage::BuildCurrentJson()
{
	BuildCurrent();
	return FReply::Handled();
}

FReply SMASkillModuleEditPage::SaveAndBuild()
{
	if (SaveJson()) BuildCurrent();
	return FReply::Handled();
}

void SMASkillModuleEditPage::OnSourceDirectoryCommitted(const FText&, ETextCommit::Type)
{
	RefreshJsonFiles();
}

void SMASkillModuleEditPage::OnJsonSelected(FMASkillModuleJsonItem Item, ESelectInfo::Type)
{
	if (bRestoringSelection || !Item || Item == SelectedJsonItem) return;
	if (!ResolvePendingChanges())
	{
		RestoreSelection();
		return;
	}
	if (!LoadJson(Item->FilePath))
	{
		if (SelectedJsonItem) LoadJson(SelectedJsonItem->FilePath);
		RestoreSelection();
		return;
	}

	SelectedJsonItem = Item;
	RestoreSelection();
}

TSharedRef<ITableRow> SMASkillModuleEditPage::GenerateJsonRow(
	FMASkillModuleJsonItem Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	FText ModuleName;
	if (Item)
	{
		if (!Item->bHeaderValid)
		{
			ModuleName = FText::FromString(FPaths::GetCleanFilename(Item->FilePath));
		}
		else
		{
			ModuleName = Item->ModuleName.IsNone()
				? LOCTEXT("UnnamedModule", "Unnamed Module")
				: FText::FromName(Item->ModuleName);
		}
	}
	const FText ModuleId = Item && Item->ModuleId > 0
		? FText::FromString(FString::Printf(TEXT("#%d"), Item->ModuleId))
		: FText::FromString(TEXT("-"));
	const FText Tooltip = Item
		? FText::FromString(Item->FilePath)
		: FText::GetEmpty();
	return SNew(STableRow<FMASkillModuleJsonItem>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(4.f, 2.f, 8.f, 2.f)
		[
			SNew(STextBlock)
				.Text(ModuleName)
				.ToolTipText(Tooltip)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 2.f, 4.f, 2.f)
		[
			SNew(STextBlock)
				.Text(ModuleId)
				.ToolTipText(Tooltip)
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
		]
	];
}

bool SMASkillModuleEditPage::LoadJson(const FString& FilePath)
{
	FMASkillModuleJsonSource Source;
	FText Error;
	if (!FMASkillModuleJsonFile::Load(FilePath, Source, Error))
	{
		ShowError(Error);
		return false;
	}

	FMASkillModuleReadResult ReadResult = FMASkillModuleJsonReader::Read(Source, *EditorObject);
	if (!ReadResult.IsValid())
	{
		ShowError(ReadResult.GetDiagnosticsText());
		return false;
	}

	EditorObject->SetModule(ReadResult.ModuleId, MoveTemp(ReadResult.ModuleData));
	SelectedJsonPath = FilePath;
	bNewJson = false;
	DetailsView->SetObject(EditorObject.Get(), true);
	bDirty = false;
	StatusText = FText::FromString(FString::Printf(TEXT("Editing %s"), *FilePath));
	return true;
}

bool SMASkillModuleEditPage::SaveJson()
{
	if (bNewJson) return SaveNewJson();
	if (SelectedJsonPath.IsEmpty())
	{
		ShowError(LOCTEXT("NoSelectedJson", "Select a module JSON before saving."));
		return false;
	}
	if (!bDirty) return true;

	FString Json;
	FText Error;
	if (!FMASkillModuleJsonWriter::Write(
		EditorObject->GetModuleId(),
		EditorObject->GetModuleData(),
		Json,
		Error)
		|| !FMASkillModuleJsonFile::Save(SelectedJsonPath, Json, true, Error))
	{
		ShowError(Error);
		return false;
	}

	bDirty = false;
	if (SelectedJsonItem)
	{
		SelectedJsonItem->ModuleId = EditorObject->GetModuleId();
		SelectedJsonItem->ModuleName = EditorObject->GetModuleData().ModuleName;
		SelectedJsonItem->bHeaderValid = true;
		JsonListView->RequestListRefresh();
	}
	StatusText = FText::FromString(FString::Printf(TEXT("Saved %s"), *SelectedJsonPath));
	return true;
}

bool SMASkillModuleEditPage::SaveNewJson()
{
	int32 ModuleId = 0;
	FText Error;
	if (!FMASkillModuleBuildPipeline::ResolveNextModuleId(SourceDirectory, ModuleId, Error))
	{
		ShowError(Error);
		return false;
	}

	const FString FilePath = FPaths::Combine(SourceDirectory, FString::Printf(TEXT("M_%d.json"), ModuleId));
	FString Json;
	if (!FMASkillModuleJsonWriter::Write(ModuleId, EditorObject->GetModuleData(), Json, Error)
		|| !FMASkillModuleJsonFile::Save(FilePath, Json, false, Error))
	{
		ShowError(Error);
		return false;
	}

	EditorObject->AssignModuleId(ModuleId);
	SelectedJsonPath = FilePath;
	bNewJson = false;
	DetailsView->ForceRefresh();
	bDirty = false;

	const FMASkillModuleJsonItem NewItem = MakeShared<FMASkillModuleJsonListItem>();
	NewItem->FilePath = FilePath;
	NewItem->ModuleId = ModuleId;
	NewItem->ModuleName = EditorObject->GetModuleData().ModuleName;
	NewItem->bHeaderValid = true;
	JsonItems.Add(NewItem);
	SortJsonItems();
	JsonListView->RequestListRefresh();
	SelectedJsonItem = NewItem;
	RestoreSelection();
	StatusText = FText::FromString(FString::Printf(TEXT("Created %s"), *FilePath));
	return true;
}

bool SMASkillModuleEditPage::BuildCurrent()
{
	if (bDirty)
	{
		ShowError(LOCTEXT("BuildRequiresSave", "Save the current module JSON before building it."));
		return false;
	}
	if (SelectedJsonPath.IsEmpty())
	{
		ShowError(LOCTEXT("BuildRequiresJson", "Select a saved module JSON before building it."));
		return false;
	}

	FMASkillModuleBuildSummary Summary;
	FText Error;
	if (!FMASkillModuleBuildPipeline::BuildFile(SourceDirectory, SelectedJsonPath, Summary, Error))
	{
		ShowError(Error);
		return false;
	}

	StatusText = Summary.Built > 0
		? LOCTEXT("BuildCurrentSucceeded", "Built the selected module asset.")
		: LOCTEXT("BuildCurrentUpToDate", "The selected module asset is up to date.");
	return true;
}

bool SMASkillModuleEditPage::CommitSourceDirectory()
{
	const FString InDirectory = SourceDirectoryTextBox->GetText().ToString();
	if (InDirectory.IsEmpty())
	{
		ShowError(LOCTEXT("SourceDirectoryNotConfigured", "JSON directory is not configured."));
		SourceDirectoryTextBox->SetText(FText::FromString(SourceDirectory));
		return false;
	}

	const FString ProjectDirectory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	FString FullDirectory = FPaths::IsRelative(InDirectory)
		? FPaths::ConvertRelativePathToFull(ProjectDirectory, InDirectory)
		: FPaths::ConvertRelativePathToFull(InDirectory);
	FPaths::NormalizeDirectoryName(FullDirectory);

	FString NormalizedProjectDirectory = ProjectDirectory;
	FPaths::NormalizeDirectoryName(NormalizedProjectDirectory);
	if (!FPaths::IsSamePath(FullDirectory, NormalizedProjectDirectory)
		&& !FPaths::IsUnderDirectory(FullDirectory, NormalizedProjectDirectory))
	{
		ShowError(LOCTEXT("SourceDirectoryOutsideProject", "JSON directory must be inside the project directory."));
		SourceDirectoryTextBox->SetText(FText::FromString(SourceDirectory));
		return false;
	}
	if (!IFileManager::Get().DirectoryExists(*FullDirectory))
	{
		ShowError(LOCTEXT("SourceDirectoryNotFound", "JSON directory does not exist."));
		SourceDirectoryTextBox->SetText(FText::FromString(SourceDirectory));
		return false;
	}

	FString RelativeDirectory = FullDirectory;
	FPaths::MakePathRelativeTo(RelativeDirectory, *ProjectDirectory);
	FPaths::NormalizeDirectoryName(RelativeDirectory);

	UMAGameSettings* Settings = GetMutableDefault<UMAGameSettings>();
	const FString PreviousDirectory = Settings->SkillModuleJsonDirectory.Path;
	if (PreviousDirectory != RelativeDirectory)
	{
		Settings->SkillModuleJsonDirectory.Path = RelativeDirectory;
		if (!Settings->TryUpdateDefaultConfigFile())
		{
			Settings->SkillModuleJsonDirectory.Path = PreviousDirectory;
			ShowError(LOCTEXT("SaveSourceDirectoryFailed", "Failed to save the JSON directory to MA Game Settings."));
			SourceDirectoryTextBox->SetText(FText::FromString(SourceDirectory));
			return false;
		}
	}

	SourceDirectory = MoveTemp(FullDirectory);
	SourceDirectoryTextBox->SetText(FText::FromString(SourceDirectory));
	return true;
}

bool SMASkillModuleEditPage::ResolvePendingChanges()
{
	if (!bDirty) return true;

	const EAppReturnType::Type Result = FMessageDialog::Open(
		EAppMsgType::YesNoCancel,
		LOCTEXT("UnsavedChanges", "Save changes to the current module JSON?"));
	if (Result == EAppReturnType::Cancel) return false;
	if (Result == EAppReturnType::No)
	{
		DiscardChanges();
		return true;
	}
	return SaveJson();
}

void SMASkillModuleEditPage::DiscardChanges()
{
	if (!bNewJson && !SelectedJsonPath.IsEmpty() && LoadJson(SelectedJsonPath)) return;

	SelectedJsonItem.Reset();
	SelectedJsonPath.Reset();
	bDirty = false;
	bNewJson = false;
	DetailsView->SetObject(nullptr);
	RestoreSelection();
	StatusText = LOCTEXT("ChangesDiscarded", "Discarded the unsaved module changes.");
}

void SMASkillModuleEditPage::RestoreSelection()
{
	bRestoringSelection = true;
	if (SelectedJsonItem) JsonListView->SetSelection(SelectedJsonItem, ESelectInfo::Direct);
	else JsonListView->ClearSelection();
	bRestoringSelection = false;
}

void SMASkillModuleEditPage::ShowError(const FText& Error)
{
	StatusText = Error;
	FMessageDialog::Open(EAppMsgType::Ok, Error);
}

#undef LOCTEXT_NAMESPACE
