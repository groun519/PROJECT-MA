#include "GAS/Skill/Module/Json/MASkillModuleDataValidator.h"

#include "GAS/Skill/Module/MASkillModuleDataTypes.h"

FText FMASkillModuleDiagnostic::ToText() const
{
	return Path.IsEmpty()
		? Message
		: FText::FromString(FString::Printf(TEXT("%s: %s"), *Path, *Message.ToString()));
}

bool FMASkillModuleDataValidator::Validate(
	const int32 ModuleId,
	const FMASkillModuleData& ModuleData,
	FMASkillModuleDiagnostic& OutDiagnostic)
{
	OutDiagnostic = FMASkillModuleDiagnostic();
	if (ModuleId <= 0)
	{
		OutDiagnostic.Path = TEXT("ModuleId");
		OutDiagnostic.Message = FText::FromString(TEXT("Must be greater than 0."));
		return false;
	}

	TSet<const UClass*> AddonClasses;
	for (int32 Index = 0; Index < ModuleData.Addons.Num(); ++Index)
	{
		const UMASkillModuleAddon* Addon = ModuleData.Addons[Index];
		if (!Addon)
		{
			OutDiagnostic.Path = FString::Printf(TEXT("Module.Addons[%d]"), Index);
			OutDiagnostic.Message = FText::FromString(TEXT("Must not be null."));
			return false;
		}
		if (AddonClasses.Contains(Addon->GetClass()))
		{
			OutDiagnostic.Path = FString::Printf(TEXT("Module.Addons[%d]"), Index);
			OutDiagnostic.Message = FText::FromString(FString::Printf(
				TEXT("Duplicates addon type '%s'."),
				*Addon->GetClass()->GetName()));
			return false;
		}
		AddonClasses.Add(Addon->GetClass());
	}
	return true;
}
