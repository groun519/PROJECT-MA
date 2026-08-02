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
	if (ModuleData.ModuleType != EMASkillModuleType::Module
		&& ModuleData.ModuleType != EMASkillModuleType::Item
		&& ModuleData.ModuleType != EMASkillModuleType::Sub)
	{
		OutDiagnostic.Path = TEXT("Module.ModuleType");
		OutDiagnostic.Message = FText::FromString(TEXT("Must specify one module type."));
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
		if (!Addon->SupportsModuleType(ModuleData.ModuleType))
		{
			OutDiagnostic.Path = FString::Printf(TEXT("Module.Addons[%d]"), Index);
			OutDiagnostic.Message = FText::FromString(FString::Printf(
				TEXT("Addon type '%s' does not support module type '%s'."),
				*Addon->GetClass()->GetName(),
				*StaticEnum<EMASkillModuleType>()->GetNameStringByValue(
					static_cast<int64>(ModuleData.ModuleType))));
			return false;
		}
		AddonClasses.Add(Addon->GetClass());
	}
	return true;
}
