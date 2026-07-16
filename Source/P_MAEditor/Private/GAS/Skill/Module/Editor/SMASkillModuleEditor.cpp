#include "GAS/Skill/Module/Editor/SMASkillModuleEditor.h"

#include "GAS/Skill/Module/Editor/SMASkillModuleBuildPage.h"
#include "GAS/Skill/Module/Editor/SMASkillModuleEditPage.h"
#include "Widgets/Input/SSegmentedControl.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "MASkillModuleEditor"

void SMASkillModuleEditor::Construct(const FArguments&)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(6.f, 6.f, 6.f, 0.f)
		[
			SNew(SSegmentedControl<EPage>)
			.Value_Lambda([this] { return ActivePage; })
			.OnValueChanged(this, &SMASkillModuleEditor::OnPageSelected)
			+ SSegmentedControl<EPage>::Slot(EPage::Edit)
			.Text(LOCTEXT("EditPage", "Edit"))
			+ SSegmentedControl<EPage>::Slot(EPage::Build)
			.Text(LOCTEXT("BuildPage", "Build"))
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SAssignNew(PageSwitcher, SWidgetSwitcher)
			.WidgetIndex(static_cast<int32>(EPage::Edit))
			+ SWidgetSwitcher::Slot()
			[
				SAssignNew(EditPage, SMASkillModuleEditPage)
			]
			+ SWidgetSwitcher::Slot()
			[
				SAssignNew(BuildPage, SMASkillModuleBuildPage)
			]
		]
	];
}

bool SMASkillModuleEditor::CanClose()
{
	return EditPage->ResolvePendingChanges();
}

void SMASkillModuleEditor::OnPageSelected(const EPage Page)
{
	if (Page == ActivePage) return;
	if (Page == EPage::Build)
	{
		if (!EditPage->ResolvePendingChanges()) return;
		if (!EditPage->CommitSourceDirectory()) return;
		BuildPage->Refresh(EditPage->GetSourceDirectory());
	}

	ActivePage = Page;
	PageSwitcher->SetActiveWidgetIndex(static_cast<int32>(Page));
}

#undef LOCTEXT_NAMESPACE
