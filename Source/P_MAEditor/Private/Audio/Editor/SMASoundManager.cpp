#include "Audio/Editor/SMASoundManager.h"

#include "AssetRegistry/AssetData.h"
#include "Audio/Gameplay/MAGameplaySoundLibrary.h"
#include "Audio/Music/MAMusicLibrary.h"
#include "Audio/Setting/MAAudioSetting.h"
#include "AudioEditorModule.h"
#include "EdGraph/EdGraphNode.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "GraphEditor.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "SGameplayTagCombo.h"
#include "ScopedTransaction.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundNodeWavePlayer.h"
#include "Styling/AppStyle.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STreeView.h"

#define LOCTEXT_NAMESPACE "MASoundManager"

SMASoundManager::~SMASoundManager()
{
	if (GEditor)
	{
		GEditor->UnregisterForUndo(this);
		GEditor->ResetPreviewAudioComponent();
	}
}

void SMASoundManager::Construct(const FArguments&)
{
	if (GEditor) GEditor->RegisterForUndo(this);
	FModuleManager::LoadModuleChecked<IAudioEditorModule>(TEXT("AudioEditor"));

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bLockable = false;
	DetailsViewArgs.bUpdatesFromSelection = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ObjectsUseNameArea;
	DetailsView = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"))
		.CreateDetailView(DetailsViewArgs);

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
			[
				SNew(SSegmentedControl<ELibrary>)
				.Value_Lambda([this] { return ActiveLibrary; })
				.OnValueChanged(this, &SMASoundManager::OnLibrarySelected)
				+ SSegmentedControl<ELibrary>::Slot(ELibrary::Gameplay)
				.Text(LOCTEXT("GameplayLibrary", "Gameplay"))
				+ SSegmentedControl<ELibrary>::Slot(ELibrary::Music)
				.Text(LOCTEXT("MusicLibrary", "Music"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.Padding(12.f, 0.f)
			[
				SNew(STextBlock)
				.Text(this, &SMASoundManager::GetActiveLibraryPath)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("StopPreview", "Stop"))
				.OnClicked(this, &SMASoundManager::StopPreview)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.OnClicked(this, &SMASoundManager::RefreshLibraries)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("Save", "Save"))
				.OnClicked(this, &SMASoundManager::SaveActiveLibrary)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(6.f, 0.f)
		[
			SNew(SSplitter)
			+ SSplitter::Slot()
			.Value(0.62f)
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
						[
							SNew(SSearchBox)
							.HintText(LOCTEXT("SearchMappings", "Search tag or sound"))
							.OnTextChanged(this, &SMASoundManager::OnSearchTextChanged)
						]
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(6.f, 0.f, 0.f, 0.f)
						[
							SNew(SButton)
							.Text(LOCTEXT("AddMapping", "+ Mapping"))
							.IsEnabled(this, &SMASoundManager::CanAddEntry)
							.OnClicked(this, &SMASoundManager::AddEntry)
						]
					]
					+ SVerticalBox::Slot()
					.FillHeight(1.f)
					[
						SAssignNew(MappingTreeView, STreeView<FMASoundManagerItem>)
						.TreeItemsSource(&MappingRoots)
						.OnGetChildren(this, &SMASoundManager::GetMappingChildren)
						.OnGenerateRow(this, &SMASoundManager::GenerateMappingRow)
						.OnSelectionChanged(this, &SMASoundManager::OnMappingSelectionChanged)
						.OnMouseButtonDoubleClick(this, &SMASoundManager::OnMappingDoubleClicked)
					]
				]
			]
			+ SSplitter::Slot()
			.Value(0.38f)
			[
				SNew(SBorder)
				.Padding(2.f)
				[
					SNew(SSplitter)
					.Orientation(Orient_Vertical)
					+ SSplitter::Slot()
					.Value(0.35f)
					[
						DetailsView.ToSharedRef()
					]
					+ SSplitter::Slot()
					.Value(0.65f)
					[
						SAssignNew(GraphPanelBox, SBox)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("NoCueSelected", "Select a SoundCue to view its graph."))
							.Justification(ETextJustify::Center)
						]
					]
				]
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f)
		[
			SNew(STextBlock)
			.Text_Lambda([this] { return StatusText; })
		]
	];

	RefreshLibraries();
}

void SMASoundManager::PostUndo(const bool)
{
	RefreshAfterUndoRedo();
}

void SMASoundManager::PostRedo(const bool)
{
	RefreshAfterUndoRedo();
}

void SMASoundManager::OnLibrarySelected(const ELibrary Library)
{
	if (Library == ActiveLibrary) return;
	ActiveLibrary = Library;
	bAddingEntry = false;
	SelectedSound.Reset();
	SelectedCue.Reset();
	SelectedGraphNode.Reset();
	ShowActiveLibrary();
}

void SMASoundManager::OnSearchTextChanged(const FText& Text)
{
	SearchText = Text.ToString().TrimStartAndEnd();
	RebuildMappingTree();
}

void SMASoundManager::OnMappingSelectionChanged(FMASoundManagerItem Item, ESelectInfo::Type)
{
	if (!Item) return;
	if (Item->Type == EMASoundManagerNodeType::Sound
		|| Item->Type == EMASoundManagerNodeType::Wave)
	{
		SelectSound(Item->Sound.Get(), Item->OwnerCue.Get(), Item->GraphNode.Get());
	}
	else if (!Item->Children.IsEmpty())
	{
		const FMASoundManagerItem& Sound = Item->Children[0];
		SelectSound(Sound->Sound.Get(), Sound->OwnerCue.Get(), Sound->GraphNode.Get());
	}
}

void SMASoundManager::OnMappingDoubleClicked(FMASoundManagerItem Item)
{
	if (!Item) return;
	if (Item->Type == EMASoundManagerNodeType::Sound
		|| Item->Type == EMASoundManagerNodeType::Wave)
	{
		OpenSoundAsset(Item);
	}
	else if (!Item->Children.IsEmpty())
	{
		OpenSoundAsset(Item->Children[0]);
	}
}

void SMASoundManager::OnMappedSoundChanged(const FAssetData& AssetData, FMASoundManagerItem Item)
{
	if (!Item || Item->Type != EMASoundManagerNodeType::Sound) return;
	USoundBase* Sound = Cast<USoundBase>(AssetData.GetAsset());
	const FScopedTransaction Transaction(LOCTEXT("SetMappedSoundTransaction", "Set mapped sound"));

	bool bChanged = false;
	if (IsGameplayLibraryActive())
	{
		if (UMAGameplaySoundLibrary* Library = Cast<UMAGameplaySoundLibrary>(GameplayLibrary.Get()))
		{
			bChanged = Library->SetSoundForEditor(Item->Tag, Sound);
		}
	}
	else if (UMAMusicLibrary* Library = Cast<UMAMusicLibrary>(MusicLibrary.Get()))
	{
		bChanged = Library->SetMusicForEditor(Item->Tag, Sound);
	}

	if (!bChanged)
	{
		StatusText = LOCTEXT("SetMappedSoundFailed", "The mapped sound could not be changed.");
		return;
	}

	SelectSound(Sound, Cast<USoundCue>(Sound));
	RebuildMappingTree();
	StatusText = LOCTEXT("SetMappedSoundSucceeded", "Updated the mapped sound.");
}

FReply SMASoundManager::AddEntry()
{
	if (!CanAddEntry()) return FReply::Handled();
	bAddingEntry = true;
	RebuildMappingTree();
	StatusText = LOCTEXT("ChooseMappingTag", "Choose a tag for the new mapping.");
	return FReply::Handled();
}

FReply SMASoundManager::CancelEntry()
{
	bAddingEntry = false;
	RebuildMappingTree();
	StatusText = LOCTEXT("CancelledMapping", "Cancelled the new sound mapping.");
	return FReply::Handled();
}

void SMASoundManager::CommitEntry(const FGameplayTag SoundTag)
{
	if (!bAddingEntry || !SoundTag.IsValid()) return;
	const FScopedTransaction Transaction(LOCTEXT("AddSoundMappingTransaction", "Add sound mapping"));

	bool bAdded = false;
	if (IsGameplayLibraryActive())
	{
		if (UMAGameplaySoundLibrary* Library = Cast<UMAGameplaySoundLibrary>(GameplayLibrary.Get()))
		{
			bAdded = Library->AddSoundEntryForEditor(SoundTag);
		}
	}
	else if (UMAMusicLibrary* Library = Cast<UMAMusicLibrary>(MusicLibrary.Get()))
	{
		bAdded = Library->AddMusicEntryForEditor(SoundTag);
	}

	if (!bAdded)
	{
		StatusText = LOCTEXT("AddMappingFailed", "The tag is already mapped or is not valid.");
		RebuildMappingTree();
		return;
	}

	bAddingEntry = false;
	RebuildMappingTree();
	StatusText = LOCTEXT("AddMappingSucceeded", "Added a sound mapping.");
}

FReply SMASoundManager::RemoveEntry(const FGameplayTag SoundTag)
{
	const FScopedTransaction Transaction(LOCTEXT("RemoveSoundMappingTransaction", "Remove sound mapping"));
	bool bRemoved = false;
	if (IsGameplayLibraryActive())
	{
		if (UMAGameplaySoundLibrary* Library = Cast<UMAGameplaySoundLibrary>(GameplayLibrary.Get()))
		{
			bRemoved = Library->RemoveSoundEntryForEditor(SoundTag);
		}
	}
	else if (UMAMusicLibrary* Library = Cast<UMAMusicLibrary>(MusicLibrary.Get()))
	{
		bRemoved = Library->RemoveMusicEntryForEditor(SoundTag);
	}

	if (bRemoved)
	{
		SelectSound(nullptr);
		RebuildMappingTree();
		StatusText = LOCTEXT("RemoveMappingSucceeded", "Removed the sound mapping.");
	}
	return FReply::Handled();
}

void SMASoundManager::RenameEntry(
	const FGameplayTag SoundTag,
	const FGameplayTag NewSoundTag)
{
	const FScopedTransaction Transaction(LOCTEXT("RenameSoundMappingTransaction", "Rename sound mapping"));
	bool bRenamed = false;
	if (IsGameplayLibraryActive())
	{
		if (UMAGameplaySoundLibrary* Library = Cast<UMAGameplaySoundLibrary>(GameplayLibrary.Get()))
		{
			bRenamed = Library->RenameSoundEntryForEditor(SoundTag, NewSoundTag);
		}
	}
	else if (UMAMusicLibrary* Library = Cast<UMAMusicLibrary>(MusicLibrary.Get()))
	{
		bRenamed = Library->RenameMusicEntryForEditor(SoundTag, NewSoundTag);
	}

	RebuildMappingTree();
	StatusText = bRenamed
		? LOCTEXT("RenameMappingSucceeded", "Changed the mapping tag.")
		: LOCTEXT("RenameMappingFailed", "The selected tag is already mapped or is not valid.");
}

FReply SMASoundManager::RefreshLibraries()
{
	GameplayLibrary.Reset();
	MusicLibrary.Reset();
	bAddingEntry = false;
	SelectedSound.Reset();
	SelectedCue.Reset();
	SelectedGraphNode.Reset();

	if (const UMAAudioSetting* AudioSetting = UMAAudioSetting::Get())
	{
		GameplayLibrary.Reset(AudioSetting->GameplaySoundLibrary.LoadSynchronous());
		MusicLibrary.Reset(AudioSetting->MusicLibrary.LoadSynchronous());
	}

	ShowActiveLibrary();
	return FReply::Handled();
}

FReply SMASoundManager::SaveActiveLibrary()
{
	UObject* Library = GetActiveLibrary();
	if (!Library)
	{
		StatusText = LOCTEXT(
			"LibraryNotConfigured",
			"The selected library is not configured in Project Settings > Audio Setting.");
		return FReply::Handled();
	}

	UPackage* Package = Library->GetOutermost();
	if (!Package->IsDirty())
	{
		StatusText = LOCTEXT("NoChanges", "There are no changes to save.");
		return FReply::Handled();
	}

	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(Package);
	const FEditorFileUtils::EPromptReturnCode Result =
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, false, false);
	StatusText = Result == FEditorFileUtils::PR_Success
		? FText::Format(LOCTEXT("SavedLibrary", "Saved {0}."), FText::FromString(Library->GetPathName()))
		: LOCTEXT("SaveFailed", "The selected library was not saved.");
	return FReply::Handled();
}

FReply SMASoundManager::PreviewSound(FMASoundManagerItem Item)
{
	if (GEditor && Item && Item->Sound.IsValid())
	{
		GEditor->PlayPreviewSound(Item->Sound.Get());
		StatusText = FText::Format(
			LOCTEXT("PreviewingSound", "Previewing {0}."),
			FText::FromString(Item->Sound->GetName()));
	}
	return FReply::Handled();
}

FReply SMASoundManager::StopPreview()
{
	if (GEditor) GEditor->ResetPreviewAudioComponent();
	StatusText = LOCTEXT("PreviewStopped", "Preview stopped.");
	return FReply::Handled();
}

FReply SMASoundManager::BrowseSoundAsset(FMASoundManagerItem Item)
{
	if (GEditor && Item && Item->Sound.IsValid())
	{
		GEditor->SyncBrowserToObject(Item->Sound.Get());
		StatusText = FText::Format(
			LOCTEXT("BrowsedSoundAsset", "Located {0} in the Content Browser."),
			FText::FromString(Item->Sound->GetName()));
	}
	return FReply::Handled();
}

FReply SMASoundManager::OpenSoundAsset(FMASoundManagerItem Item)
{
	if (GEditor && Item && Item->Sound.IsValid())
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Item->Sound.Get());
		StatusText = FText::Format(
			LOCTEXT("OpenedSoundAsset", "Opened {0}."),
			FText::FromString(Item->Sound->GetName()));
	}
	return FReply::Handled();
}

void SMASoundManager::ShowActiveLibrary()
{
	RebuildMappingTree();
	SelectSound(SelectedSound.Get(), SelectedCue.Get(), SelectedGraphNode.Get());
	UObject* Library = GetActiveLibrary();
	StatusText = Library
		? FText::Format(LOCTEXT("EditingLibrary", "Editing {0}."), FText::FromString(Library->GetPathName()))
		: LOCTEXT(
			"MissingLibrary",
			"The selected library is not configured in Project Settings > Audio Setting.");
}

void SMASoundManager::RebuildMappingTree()
{
	MappingRoots.Reset();
	TArray<FGameplayTag> SoundTags;

	if (IsGameplayLibraryActive())
	{
		if (const UMAGameplaySoundLibrary* Library = Cast<UMAGameplaySoundLibrary>(GameplayLibrary.Get()))
		{
			Library->GetSoundTagsForEditor(SoundTags);
			for (const FGameplayTag SoundTag : SoundTags)
			{
				FMASoundManagerItem Entry = MakeShared<FMASoundManagerNode>();
				Entry->Type = EMASoundManagerNodeType::Entry;
				Entry->Tag = SoundTag;
				Entry->Label = SoundTag.ToString();

				Entry->Children.Add(MakeMappedSoundNode(SoundTag, Library->FindSound(SoundTag)));

				if (MatchesSearch(Entry)) MappingRoots.Add(MoveTemp(Entry));
			}
		}
	}
	else if (const UMAMusicLibrary* Library = Cast<UMAMusicLibrary>(MusicLibrary.Get()))
	{
		Library->GetMusicTagsForEditor(SoundTags);
		for (const FGameplayTag MusicTag : SoundTags)
		{
			FMASoundManagerItem Entry = MakeShared<FMASoundManagerNode>();
			Entry->Type = EMASoundManagerNodeType::Entry;
			Entry->Tag = MusicTag;
			Entry->Label = MusicTag.ToString();
			Entry->Children.Add(MakeMappedSoundNode(MusicTag, Library->FindMusic(MusicTag)));
			if (MatchesSearch(Entry)) MappingRoots.Add(MoveTemp(Entry));
		}
	}

	MappingRoots.Sort([](const FMASoundManagerItem& A, const FMASoundManagerItem& B)
	{
		return A->Label < B->Label;
	});

	if (bAddingEntry && GetActiveLibrary())
	{
		FMASoundManagerItem Draft = MakeShared<FMASoundManagerNode>();
		Draft->Type = EMASoundManagerNodeType::DraftEntry;
		Draft->Label = TEXT("New Mapping");
		MappingRoots.Insert(MoveTemp(Draft), 0);
	}

	if (!MappingTreeView) return;
	MappingTreeView->RequestTreeRefresh();
	for (const FMASoundManagerItem& Entry : MappingRoots)
	{
		MappingTreeView->SetItemExpansion(Entry, true);
		for (const FMASoundManagerItem& Sound : Entry->Children)
		{
			if (!Sound->Children.IsEmpty()) MappingTreeView->SetItemExpansion(Sound, true);
		}
	}
}

void SMASoundManager::SelectSound(
	USoundBase* Sound,
	USoundCue* OwnerCue,
	UEdGraphNode* GraphNode)
{
	SelectedSound = Sound;
	SelectedCue = Cast<USoundCue>(Sound) ? Cast<USoundCue>(Sound) : OwnerCue;
	SelectedGraphNode = GraphNode;
	if (!GraphPanelBox || !DetailsView) return;

	DetailsView->SetObject(Sound, true);
	USoundCue* SoundCue = SelectedCue.Get();
	if (SoundCue && SoundCue->GetGraph())
	{
		if (DisplayedCue.Get() != SoundCue || !CueGraphEditor.IsValid())
		{
			FGraphAppearanceInfo AppearanceInfo;
			AppearanceInfo.CornerText = LOCTEXT("SoundCueGraph", "SOUND CUE");
			CueGraphEditor =
				SNew(SGraphEditor)
				.IsEditable(false)
				.DisplayAsReadOnly(false)
				.Appearance(AppearanceInfo)
				.GraphToEdit(SoundCue->GetGraph())
				.AutoExpandActionMenu(false)
				.ShowGraphStateOverlay(false);
			GraphPanelBox->SetContent(CueGraphEditor.ToSharedRef());
			DisplayedCue = SoundCue;
		}

		if (GraphNode && GraphNode->GetGraph() == SoundCue->GetGraph())
		{
			CueGraphEditor->ClearSelectionSet();
			CueGraphEditor->SetNodeSelection(GraphNode, true);
			CueGraphEditor->JumpToNode(GraphNode);
		}
		else
		{
			CueGraphEditor->ClearSelectionSet();
		}
		return;
	}

	CueGraphEditor.Reset();
	DisplayedCue.Reset();
	GraphPanelBox->SetContent(
		SNew(STextBlock)
		.Text(LOCTEXT("NoOwningCue", "The selected sound does not belong to a SoundCue."))
		.Justification(ETextJustify::Center));
}

void SMASoundManager::RefreshAfterUndoRedo()
{
	RebuildMappingTree();
	SelectSound(SelectedSound.Get(), SelectedCue.Get(), SelectedGraphNode.Get());
	StatusText = LOCTEXT("UndoRedoApplied", "Updated the sound mappings after undo or redo.");
}

FMASoundManagerItem SMASoundManager::MakeMappedSoundNode(
	const FGameplayTag SoundTag,
	USoundBase* Sound) const
{
	FMASoundManagerItem Item = MakeShared<FMASoundManagerNode>();
	Item->Type = EMASoundManagerNodeType::Sound;
	Item->Tag = SoundTag;
	Item->Sound = Sound;
	Item->Label = Sound ? Sound->GetName() : TEXT("Missing Sound");

	if (USoundCue* SoundCue = Cast<USoundCue>(Sound))
	{
		Item->OwnerCue = SoundCue;
		TArray<USoundNodeWavePlayer*> WavePlayers;
		SoundCue->RecursiveFindNode<USoundNodeWavePlayer>(SoundCue->FirstNode, WavePlayers);
		for (const USoundNodeWavePlayer* WavePlayer : WavePlayers)
		{
			FMASoundManagerItem Wave = MakeShared<FMASoundManagerNode>();
			Wave->Type = EMASoundManagerNodeType::Wave;
			Wave->Sound = WavePlayer ? WavePlayer->GetSoundWave() : nullptr;
			Wave->OwnerCue = SoundCue;
			Wave->GraphNode = WavePlayer ? WavePlayer->GetGraphNode() : nullptr;
			Wave->Label = Wave->Sound.IsValid() ? Wave->Sound->GetName() : TEXT("Missing Wave");
			Item->Children.Add(MoveTemp(Wave));
		}
	}

	return Item;
}

bool SMASoundManager::MatchesSearch(const FMASoundManagerItem& Item) const
{
	if (SearchText.IsEmpty()) return true;
	if (Item->Label.Contains(SearchText, ESearchCase::IgnoreCase)) return true;
	return Item->Children.ContainsByPredicate([this](const FMASoundManagerItem& Child)
	{
		return MatchesSearch(Child);
	});
}

void SMASoundManager::GetMappingChildren(
	FMASoundManagerItem Item,
	TArray<FMASoundManagerItem>& OutChildren) const
{
	if (Item) OutChildren.Append(Item->Children);
}

TSharedRef<ITableRow> SMASoundManager::GenerateMappingRow(
	FMASoundManagerItem Item,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	if (Item && Item->Type == EMASoundManagerNodeType::DraftEntry)
	{
		return SNew(STableRow<FMASoundManagerItem>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.Padding(4.f, 2.f)
			[
				SNew(SGameplayTagCombo)
				.Filter(GetTagFilter())
				.SettingsName(IsGameplayLibraryActive()
					? TEXT("MASoundManager.NewGameplayEntry")
					: TEXT("MASoundManager.NewMusicEntry"))
				.Tag(FGameplayTag())
				.OnTagChanged(this, &SMASoundManager::CommitEntry)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2.f)
			[
				PropertyCustomizationHelpers::MakeDeleteButton(
					FSimpleDelegate::CreateLambda([this] { CancelEntry(); }),
					LOCTEXT("CancelMapping", "Cancel mapping"))
			]
		];
	}

	if (Item && Item->Type == EMASoundManagerNodeType::Entry)
	{
		return SNew(STableRow<FMASoundManagerItem>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.Padding(4.f, 2.f)
			[
				SNew(SGameplayTagCombo)
				.Filter(GetTagFilter())
				.SettingsName(IsGameplayLibraryActive()
					? TEXT("MASoundManager.GameplayEntry")
					: TEXT("MASoundManager.MusicEntry"))
				.Tag(Item->Tag)
				.OnTagChanged_Lambda([this, Item](const FGameplayTag NewTag)
				{
					RenameEntry(Item->Tag, NewTag);
				})
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2.f)
			[
				PropertyCustomizationHelpers::MakeDeleteButton(
					FSimpleDelegate::CreateLambda([this, Item] { RemoveEntry(Item->Tag); }),
					LOCTEXT("RemoveMapping", "Remove mapping"))
			]
		];
	}

	if (Item && Item->Type == EMASoundManagerNodeType::Wave)
	{
		const bool bValidSound = Item->Sound.IsValid();
		return SNew(STableRow<FMASoundManagerItem>, OwnerTable)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(4.f, 2.f, 8.f, 2.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("WaveLabel", "Wave"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			.Padding(0.f, 2.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Item->Label))
				.ColorAndOpacity(bValidSound ? FSlateColor::UseForeground() : FLinearColor::Red)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2.f)
			[
				PropertyCustomizationHelpers::MakeCustomButton(
					FAppStyle::GetBrush("MediaAsset.AssetActions.Play.Small"),
					FSimpleDelegate::CreateLambda([this, Item] { PreviewSound(Item); }),
					LOCTEXT("PreviewWave", "Play sound"),
					bValidSound)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2.f)
			[
				PropertyCustomizationHelpers::MakeBrowseButton(
					FSimpleDelegate::CreateLambda([this, Item] { BrowseSoundAsset(Item); }),
					LOCTEXT("BrowseWave", "Browse to asset in Content Browser"),
					bValidSound)
			]
		];
	}

	USoundBase* Sound = Item ? Item->Sound.Get() : nullptr;
	return SNew(STableRow<FMASoundManagerItem>, OwnerTable)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(4.f, 2.f)
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(USoundBase::StaticClass())
			.ObjectPath(Sound ? Sound->GetPathName() : FString())
			.AllowClear(true)
			.AllowCreate(false)
			.DisplayThumbnail(false)
			.OnObjectChanged_Lambda([this, Item](const FAssetData& AssetData)
			{
				OnMappedSoundChanged(AssetData, Item);
			})
		]
	];
}

UObject* SMASoundManager::GetActiveLibrary() const
{
	return IsGameplayLibraryActive() ? GameplayLibrary.Get() : MusicLibrary.Get();
}

FText SMASoundManager::GetActiveLibraryPath() const
{
	const UObject* Library = GetActiveLibrary();
	return Library
		? FText::FromString(Library->GetPathName())
		: LOCTEXT("UnconfiguredLibrary", "Not configured");
}

FString SMASoundManager::GetTagFilter() const
{
	return IsGameplayLibraryActive() ? TEXT("GameplayCue.Hit,Sound") : TEXT("Music");
}

bool SMASoundManager::IsGameplayLibraryActive() const
{
	return ActiveLibrary == ELibrary::Gameplay;
}

bool SMASoundManager::CanAddEntry() const
{
	return GetActiveLibrary() && !bAddingEntry;
}

#undef LOCTEXT_NAMESPACE
