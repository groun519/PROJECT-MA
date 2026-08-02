#include "GAS/Skill/Definition/Assembly/MASkillAddonAssembler.h"

#include "GAS/Skill/Addon/MASkillModuleAddon.h"
#include "GAS/Skill/Module/MASkillModule.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"

void FMASkillAddonAssembler::AppendFrom(
	UObject& ResultOuter,
	FMASkillModuleData& ResultData,
	const UMASkillModule& SourceModule,
	const EMASkillAddonAssemblyStage Stage,
	const FMASkillScopes& SourceScopes)
{
	SourceModule.ForEachAddon([&](const UMASkillModuleAddon& SourceAddon)
	{
		UMASkillModuleAddon* ResultAddon = nullptr;
		for (UMASkillModuleAddon* Candidate : ResultData.Addons)
		{
			if (Candidate->GetClass() == SourceAddon.GetClass())
			{
				ResultAddon = Candidate;
				break;
			}
		}

		UMASkillModuleAddon* AssembledAddon = SourceAddon.AssembleInto(
			ResultOuter,
			ResultAddon,
			Stage,
			SourceScopes);
		if (!ResultAddon && AssembledAddon)
		{
			ResultData.Addons.Add(AssembledAddon);
		}
	});
}

void FMASkillAddonAssembler::Finalize(
	FMASkillModuleData& ResultData,
	const EMASkillAddonAssemblyStage Stage)
{
	for (int32 Index = 0; Index < ResultData.Addons.Num();)
	{
		if (!ResultData.Addons[Index]->Finalize(Stage))
		{
			ResultData.Addons.RemoveAt(Index);
			continue;
		}
		++Index;
	}
}
