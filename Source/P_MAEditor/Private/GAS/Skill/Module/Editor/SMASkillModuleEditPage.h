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
template<typename ItemType> class SListView;

struct FMASkillModuleJsonListItem
{
	FString FilePath;
	int32 ModuleId = 0;
	FName ModuleName = NAME_None;
	bool bHeaderValid = false;
};

using FMASkillModuleJsonItem = TSharedPtr<FMASkillModuleJsonListItem>;

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
	void SortJsonItems();
	FReply NewJson();
	FReply SaveCurrentJson();
	FReply BuildCurrentJson();
	FReply SaveAndBuild();

	void OnSourceDirectoryCommitted(const FText& Text, ETextCommit::Type CommitType);
	void OnJsonSelected(FMASkillModuleJsonItem Item, ESelectInfo::Type SelectInfo);
	TSharedRef<ITableRow> GenerateJsonRow(
		FMASkillModuleJsonItem Item,
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

	TStrongObjectPtr<UMASkillModuleEditorObject> EditorObject;
	TArray<FMASkillModuleJsonItem> JsonItems;
	FMASkillModuleJsonItem SelectedJsonItem;
	TSharedPtr<SListView<FMASkillModuleJsonItem>> JsonListView;
	TSharedPtr<SEditableTextBox> SourceDirectoryTextBox;
	TSharedPtr<IDetailsView> DetailsView;
};
