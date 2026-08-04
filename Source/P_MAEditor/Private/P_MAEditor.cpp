#include "Framework/Docking/TabManager.h"
#include "GAS/Skill/Module/Editor/SMASkillModuleEditor.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "P_MAEditor"

static const FName SkillModuleEditorTabName(TEXT("SkillModuleEditor"));

static TSharedRef<SDockTab> SpawnSkillModuleEditor(const FSpawnTabArgs&)
{
	const TSharedRef<SMASkillModuleEditor> ModuleEditor = SNew(SMASkillModuleEditor);
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.OnCanCloseTab(SDockTab::FCanCloseTab::CreateSP(ModuleEditor, &SMASkillModuleEditor::CanClose))
		[
			ModuleEditor
		];
}

class FP_MAEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		ProjectMAWorkspaceGroup = WorkspaceMenu::GetMenuStructure().GetToolsCategory()->AddGroup(
			TEXT("ProjectMA"),
			LOCTEXT("ProjectMAWorkspaceGroup", "PROJECT-MA"),
			FSlateIcon(),
			true);

		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
			SkillModuleEditorTabName,
			FOnSpawnTab::CreateStatic(&SpawnSkillModuleEditor))
			.SetDisplayName(LOCTEXT("SkillModuleEditor", "Skill Module Editor"))
			.SetTooltipText(LOCTEXT("SkillModuleEditorTooltip", "Open the JSON-backed skill module editor."))
			.SetGroup(ProjectMAWorkspaceGroup.ToSharedRef())
			.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("ClassIcon.DataTable")));
	}

	virtual void ShutdownModule() override
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(SkillModuleEditorTabName);
		if (ProjectMAWorkspaceGroup.IsValid())
		{
			WorkspaceMenu::GetMenuStructure().GetToolsCategory()->RemoveItem(ProjectMAWorkspaceGroup.ToSharedRef());
			ProjectMAWorkspaceGroup.Reset();
		}
	}

private:
	TSharedPtr<FWorkspaceItem> ProjectMAWorkspaceGroup;
};

IMPLEMENT_MODULE(FP_MAEditorModule, P_MAEditor)

#undef LOCTEXT_NAMESPACE
