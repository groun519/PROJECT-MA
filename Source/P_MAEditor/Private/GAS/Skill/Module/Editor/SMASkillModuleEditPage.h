#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/Editor/MASkillModuleEditorObject.h"
#include "Misc/NotifyHook.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/SCompoundWidget.h"

class IDetailsView;
class ITableRow;
class SEditableTextBox;
class STableViewBase;
template<typename ItemType> class STreeView;

struct FMASkillModuleTreeNode
{
	FString FilePath;
	int32 ModuleId = 0;
	FName ModuleName = NAME_None;
	EMASkillModuleType ModuleType = EMASkillModuleType::None;
	FText Error;
	bool bHeaderValid = false;
	bool bSourcePathValid = false;
	bool bGroup = false;
	TArray<TSharedPtr<FMASkillModuleTreeNode>> Children;

	bool IsValidSource() const { return bHeaderValid && bSourcePathValid; }
};

using FMASkillModuleTreeItem = TSharedPtr<FMASkillModuleTreeNode>;

class SMASkillModuleEditPage : public SCompoundWidget, public FNotifyHook
{
public:
	SLATE_BEGIN_ARGS(SMASkillModuleEditPage) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	bool ResolvePendingChanges();
	bool CommitSourceDirectory();
	const FString& GetSourceDirectory() const { return SourceDirectory; }

private:
	virtual void NotifyPostChange(
		const FPropertyChangedEvent& PropertyChangedEvent,
		FProperty* PropertyThatChanged) override;

	FReply BrowseSourceDirectory();
	FReply RefreshJsonFiles();
	void SortModuleItems();
	void RebuildJsonTree();
	FReply NewJson();
	FReply SaveCurrentJson();
	FReply BuildCurrentJson();
	FReply SaveAndBuild();

	void OnSourceDirectoryCommitted(const FText& Text, ETextCommit::Type CommitType);
	void OnSearchTextChanged(const FText& Text);
	void OnJsonSelected(FMASkillModuleTreeItem Item, ESelectInfo::Type SelectInfo);
	void GetJsonChildren(
		FMASkillModuleTreeItem Item,
		TArray<FMASkillModuleTreeItem>& OutChildren) const;
	TSharedRef<ITableRow> GenerateJsonRow(
		FMASkillModuleTreeItem Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	bool LoadJson(const FString& FilePath);
	bool SaveJson();
	bool SaveNewJson();
	bool BuildCurrent();
	void DiscardChanges();
	void RestoreSelection();
	void ShowError(const FText& Error);

	FString SourceDirectory;
	FString SelectedJsonPath;
	FText StatusText;
	bool bDirty = false;
	bool bNewJson = false;
	bool bRestoringSelection = false;
	FString SearchText;

	TStrongObjectPtr<UMASkillModuleEditorObject> EditorObject;
	TArray<FMASkillModuleTreeItem> ModuleItems;
	TArray<FMASkillModuleTreeItem> RootItems;
	FMASkillModuleTreeItem SelectedModuleItem;
	TSharedPtr<STreeView<FMASkillModuleTreeItem>> JsonTreeView;
	TSharedPtr<SEditableTextBox> SourceDirectoryTextBox;
	TSharedPtr<IDetailsView> DetailsView;
};
