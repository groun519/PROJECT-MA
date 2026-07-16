#pragma once

#include "CoreMinimal.h"
#include "GAS/Skill/Module/Build/MASkillModuleBuildTypes.h"
#include "Widgets/SCompoundWidget.h"

class ITableRow;
class STableViewBase;
template<typename ItemType> class SListView;

using FMASkillModuleBuildListItem = TSharedPtr<FMASkillModuleBuildItem>;

class SMASkillModuleBuildPage : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMASkillModuleBuildPage) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	void Refresh(const FString& InSourceDirectory);

private:
	FReply RefreshStatus();
	FReply BuildSelected();
	FReply BuildRequired();
	FReply RebuildAll();
	FReply DeleteGeneratedAsset(FMASkillModuleBuildListItem Item);

	TSharedRef<ITableRow> GenerateBuildRow(
		FMASkillModuleBuildListItem Item,
		const TSharedRef<STableViewBase>& OwnerTable);

	bool RefreshItems();
	void ExecuteBuild(const TArray<FString>& SourceFiles, EMASkillModuleBuildMode BuildMode);
	void ShowError(const FText& Error);

	FString SourceDirectory;
	FText StatusText;
	TArray<FMASkillModuleBuildListItem> BuildItems;
	TSharedPtr<SListView<FMASkillModuleBuildListItem>> BuildListView;
	bool bHasBuildSources = false;
	bool bHasRequiredBuilds = false;
};
