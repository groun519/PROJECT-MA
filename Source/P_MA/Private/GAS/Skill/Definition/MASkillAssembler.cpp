#include "GAS/Skill/Definition/MASkillAssembler.h"

#include "GAS/Skill/Definition/MASkillDefinition.h"

UMASkillDefinition* FMASkillAssembler::Assemble(UObject* Outer, const TArray<TObjectPtr<UMASkillDefinition>>& OrderedDefinitions)
{
	if (!Outer) return nullptr;

	UMASkillDefinition* AssembledDefinition = nullptr;
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
	}

	return AssembledDefinition;
}
