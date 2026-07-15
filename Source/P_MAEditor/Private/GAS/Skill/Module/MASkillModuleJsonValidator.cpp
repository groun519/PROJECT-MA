#include "GAS/Skill/Module/MASkillModuleJsonValidator.h"

#include "GAS/Skill/Module/MASkillModuleDataTypes.h"

bool FMASkillModuleJsonValidator::Validate(
	const int32 ModuleId,
	const FMASkillModuleData& ModuleData,
	FText& OutError)
{
	if (ModuleId <= 0)
	{
		OutError = FText::FromString(TEXT("ModuleId must be greater than 0."));
		return false;
	}

	TSet<const UClass*> AddonClasses;
	for (int32 Index = 0; Index < ModuleData.Addons.Num(); ++Index)
	{
		const UMASkillModuleAddon* Addon = ModuleData.Addons[Index];
		if (!Addon)
		{
			OutError = FText::FromString(FString::Printf(TEXT("Module.Addons[%d] is null."), Index));
			return false;
		}
		if (AddonClasses.Contains(Addon->GetClass()))
		{
			OutError = FText::FromString(FString::Printf(
				TEXT("Module.Addons contains duplicate type '%s'."),
				*Addon->GetClass()->GetName()));
			return false;
		}
		AddonClasses.Add(Addon->GetClass());
	}
	return true;
}
