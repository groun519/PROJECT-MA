#include "GAS/Skill/Definition/MASkillAssembler.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"

UMASkillDefinition* FMASkillAssembler::Assemble(UObject* Outer, const TArray<TObjectPtr<UMASkillDefinition>>& OrderedDefinitions)
{
	if (!Outer) return nullptr;

	UMASkillDefinition* AssembledDefinition = nullptr;
	TMap<int32, FText> NameKeywordsByPriority;
	int32 PriorityOneIconCount = 0;
	bool bHasIconColors = false;

	for (UMASkillDefinition* Definition : OrderedDefinitions)
	{
		if (!Definition) continue;

		if (!AssembledDefinition)
		{
			AssembledDefinition = NewObject<UMASkillDefinition>(Outer);
			if (!AssembledDefinition) return nullptr;
			AssembledDefinition->ResetAssemblyData();
		}

		AssembledDefinition->AppendFrom(*Definition);

		const FMASkillDefinitionDisplayData& DisplayData = Definition->DisplayData;
		const FMASkillDefinitionIconData& IconData = DisplayData.IconData;
		if (IconData.Priority == 1 && IconData.Icon)
		{
			if (PriorityOneIconCount == 0)
			{
				AssembledDefinition->DisplayData.IconData.Icon = IconData.Icon;
			}
			else if (PriorityOneIconCount == 1)
			{
				AssembledDefinition->AssembledSubIcon = IconData.Icon;
			}
			++PriorityOneIconCount;
		}
		if (!bHasIconColors && IconData.Priority == 2)
		{
			AssembledDefinition->DisplayData.IconData.IconColor = IconData.IconColor;
			AssembledDefinition->DisplayData.IconData.InnerColor = IconData.InnerColor;
			bHasIconColors = true;
		}

		const FMASkillDefinitionNameData& NameData = DisplayData.NameData;
		if (NameData.Priority > 0 && !NameData.Keyword.IsEmpty())
		{
			NameKeywordsByPriority.FindOrAdd(NameData.Priority, NameData.Keyword);
		}
	}

	if (AssembledDefinition && !NameKeywordsByPriority.IsEmpty())
	{
		TArray<int32> Priorities;
		NameKeywordsByPriority.GetKeys(Priorities);
		Priorities.Sort();

		FString AssembledName;
		for (const int32 Priority : Priorities)
		{
			const FText* Keyword = NameKeywordsByPriority.Find(Priority);
			if (!Keyword || Keyword->IsEmpty()) continue;

			if (!AssembledName.IsEmpty())
			{
				AssembledName.AppendChar(TEXT(' '));
			}
			AssembledName.Append(Keyword->ToString());
		}

		AssembledDefinition->DisplayData.DisplayName = FText::FromString(AssembledName);
		AssembledDefinition->DisplayData.NameData.Keyword = FText::FromString(AssembledName);
	}

	return AssembledDefinition;
}
