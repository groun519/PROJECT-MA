#pragma once

#include "CoreMinimal.h"
#include "EditorUndoClient.h"
#include "GameplayTagContainer.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class ITableRow;
class IDetailsView;
class SBox;
class SGraphEditor;
class STableViewBase;
class UEdGraphNode;
class USoundBase;
class USoundCue;
template<typename ItemType>
class STreeView;
struct FAssetData;

enum class EMASoundManagerNodeType : uint8
{
	DraftEntry,
	Entry,
	Sound,
	Wave
};

struct FMASoundManagerNode
{
	EMASoundManagerNodeType Type = EMASoundManagerNodeType::Entry;
	FGameplayTag Tag;
	FString Label;
	TWeakObjectPtr<USoundBase> Sound;
	TWeakObjectPtr<USoundCue> OwnerCue;
	TWeakObjectPtr<UEdGraphNode> GraphNode;
	TArray<TSharedPtr<FMASoundManagerNode>> Children;
};

using FMASoundManagerItem = TSharedPtr<FMASoundManagerNode>;

class SMASoundManager : public SCompoundWidget, public FEditorUndoClient
{
public:
	SLATE_BEGIN_ARGS(SMASoundManager) {}
	SLATE_END_ARGS()

	~SMASoundManager();
	void Construct(const FArguments& InArgs);

	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override;

private:
	enum class ELibrary : uint8
	{
		Gameplay,
		Music
	};

	void OnLibrarySelected(ELibrary Library);
	void OnSearchTextChanged(const FText& Text);
	void OnMappingSelectionChanged(FMASoundManagerItem Item, ESelectInfo::Type SelectInfo);
	void OnMappingDoubleClicked(FMASoundManagerItem Item);
	void OnMappedSoundChanged(const FAssetData& AssetData, FMASoundManagerItem Item);

	FReply AddEntry();
	FReply CancelEntry();
	void CommitEntry(FGameplayTag SoundTag);
	FReply RemoveEntry(FGameplayTag SoundTag);
	void RenameEntry(FGameplayTag SoundTag, FGameplayTag NewSoundTag);

	FReply RefreshLibraries();
	FReply SaveActiveLibrary();
	FReply PreviewSound(FMASoundManagerItem Item);
	FReply StopPreview();
	FReply BrowseSoundAsset(FMASoundManagerItem Item);
	FReply OpenSoundAsset(FMASoundManagerItem Item);

	void ShowActiveLibrary();
	void RebuildMappingTree();
	void SelectSound(
		USoundBase* Sound,
		USoundCue* OwnerCue = nullptr,
		UEdGraphNode* GraphNode = nullptr);
	void RefreshAfterUndoRedo();

	FMASoundManagerItem MakeMappedSoundNode(
		FGameplayTag SoundTag,
		USoundBase* Sound) const;
	bool MatchesSearch(const FMASoundManagerItem& Item) const;
	void GetMappingChildren(FMASoundManagerItem Item, TArray<FMASoundManagerItem>& OutChildren) const;
	TSharedRef<ITableRow> GenerateMappingRow(
		FMASoundManagerItem Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	UObject* GetActiveLibrary() const;
	FText GetActiveLibraryPath() const;
	FString GetTagFilter() const;
	bool IsGameplayLibraryActive() const;
	bool CanAddEntry() const;

	TSharedPtr<SBox> GraphPanelBox;
	TSharedPtr<SGraphEditor> CueGraphEditor;
	TSharedPtr<STreeView<FMASoundManagerItem>> MappingTreeView;
	TSharedPtr<IDetailsView> DetailsView;
	TArray<FMASoundManagerItem> MappingRoots;
	TStrongObjectPtr<UObject> GameplayLibrary;
	TStrongObjectPtr<UObject> MusicLibrary;
	TWeakObjectPtr<USoundBase> SelectedSound;
	TWeakObjectPtr<USoundCue> SelectedCue;
	TWeakObjectPtr<USoundCue> DisplayedCue;
	TWeakObjectPtr<UEdGraphNode> SelectedGraphNode;
	ELibrary ActiveLibrary = ELibrary::Gameplay;
	bool bAddingEntry = false;
	FString SearchText;
	FText StatusText;
};
