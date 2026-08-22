#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMASkillModuleBuildPage;
class SMASkillModuleEditPage;
class SMASkillModulePoolPage;
class SWidgetSwitcher;

class SMASkillModuleEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMASkillModuleEditor) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	bool CanClose();

private:
	enum class EPage : uint8
	{
		Edit,
		Build,
		Pool
	};

	void OnPageSelected(EPage Page);

	TSharedPtr<SMASkillModuleEditPage> EditPage;
	TSharedPtr<SMASkillModuleBuildPage> BuildPage;
	TSharedPtr<SMASkillModulePoolPage> PoolPage;
	TSharedPtr<SWidgetSwitcher> PageSwitcher;
	EPage ActivePage = EPage::Edit;
};
