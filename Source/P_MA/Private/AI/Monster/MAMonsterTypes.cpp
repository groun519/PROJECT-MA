#include "AI/Monster/MAMonsterTypes.h"

#include "GAS/Skill/Addon/Sequence/MASkillModuleSequenceAddon.h"
#include "GAS/Skill/MASkillSystemTypes.h"
#include "GAS/Skill/Module/MASkillModule.h"
#if WITH_EDITOR
#include "GAS/Skill/Sequence/Variants/MASkillSequenceModifier_Windup.h"
#endif

bool FMonsterSkillPatternRow::LoadModuleGroups(
	TArray<FMASkillModuleGroup>& OutModuleGroups) const
{
	OutModuleGroups.Reset(Modules.Num());

	bool bLoadedAnyModule = false;
	int32 WindupTargetModuleIndex = INDEX_NONE;
	for (const TSoftObjectPtr<UMASkillModule>& ModuleAsset : Modules)
	{
		FMASkillModuleGroup& ModuleGroup = OutModuleGroups.AddDefaulted_GetRef();
		ModuleGroup.RootModule = ModuleAsset.LoadSynchronous();
		bLoadedAnyModule |= ModuleGroup.RootModule != nullptr;

		const UMASkillModuleSequenceAddon* SequenceAddon = ModuleGroup.RootModule
			? ModuleGroup.RootModule->FindAddon<UMASkillModuleSequenceAddon>()
			: nullptr;
		if (WindupDuration > 0.f
			&& WindupTargetModuleIndex == INDEX_NONE
			&& SequenceAddon
			&& !SequenceAddon->GetSequences().IsEmpty())
		{
			WindupTargetModuleIndex = OutModuleGroups.Num() - 1;
		}
	}

	if (WindupTargetModuleIndex != INDEX_NONE && WindupSubModule)
	{
		OutModuleGroups[WindupTargetModuleIndex].SubModules.Add(WindupSubModule);
	}
	return bLoadedAnyModule;
}

#if WITH_EDITOR
void FMonsterSkillPatternRow::OnDataTableChanged(
	const UDataTable* InDataTable,
	const FName)
{
	if (WindupDuration <= 0.f)
	{
		WindupSubModule = nullptr;
		return;
	}

	UMASkillModule* GeneratedModule = NewObject<UMASkillModule>(
		const_cast<UDataTable*>(InDataTable),
		NAME_None,
		RF_Transactional);
	UMASkillModuleSequenceAddon* SequenceAddon = NewObject<UMASkillModuleSequenceAddon>(
		GeneratedModule,
		NAME_None,
		RF_Transactional);
	UMASkillSequenceModifier_Windup* WindupModifier =
		NewObject<UMASkillSequenceModifier_Windup>(
			SequenceAddon,
			NAME_None,
			RF_Transactional);
	WindupModifier->Configure(WindupDuration);
	SequenceAddon->AddGeneratedModifier(*WindupModifier);

	FMASkillModuleData ModuleData;
	ModuleData.ModuleType = EMASkillModuleType::Sub;
	ModuleData.Addons.Add(SequenceAddon);
	GeneratedModule->SetGeneratedData(0, MoveTemp(ModuleData), FString());
	WindupSubModule = GeneratedModule;
}
#endif
