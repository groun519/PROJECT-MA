#include "GAS/Skill/Module/Json/MASkillModuleDataValidator.h"

#include "GAS/Skill/Action/MASkillAction.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventBindingAddon.h"
#include "GAS/Skill/Addon/Item/MASkillModuleItemAddon.h"
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
	bool bHasItemAddon = false;
	bool bHasItemUseBinding = false;
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

		if (const UMASkillModuleEventBindingAddon* EventBindingAddon =
			Cast<UMASkillModuleEventBindingAddon>(Addon))
		{
			const TArray<FMASkillEventBinding>& EventBindings = EventBindingAddon->GetEventBindings();
			for (int32 BindingIndex = 0; BindingIndex < EventBindings.Num(); ++BindingIndex)
			{
				const FMASkillEventBinding& Binding = EventBindings[BindingIndex];
				if (ModuleData.ModuleType == EMASkillModuleType::Item
					&& !Binding.Action)
				{
					OutDiagnostic.Path = FString::Printf(
						TEXT("Module.Addons[%d].EventBindings[%d].Action"),
						Index,
						BindingIndex);
					OutDiagnostic.Message = FText::FromString(
						TEXT("Item event bindings require an action."));
					return false;
				}
				if (Binding.Action && !Binding.Action->SupportsModuleType(ModuleData.ModuleType))
				{
					OutDiagnostic.Path = FString::Printf(
						TEXT("Module.Addons[%d].EventBindings[%d].Action"),
						Index,
						BindingIndex);
					OutDiagnostic.Message = FText::FromString(FString::Printf(
						TEXT("Action type '%s' does not support module type '%s'."),
						*Binding.Action->GetClass()->GetName(),
						*StaticEnum<EMASkillModuleType>()->GetNameStringByValue(
							static_cast<int64>(ModuleData.ModuleType))));
					return false;
				}
				if (ModuleData.ModuleType == EMASkillModuleType::Item
					&& Binding.BindingScope != EMASkillEventBindingScope::Global)
				{
					OutDiagnostic.Path = FString::Printf(
						TEXT("Module.Addons[%d].EventBindings[%d].BindingScope"),
						Index,
						BindingIndex);
					OutDiagnostic.Message = FText::FromString(
						TEXT("Item event bindings must use Global scope."));
					return false;
				}
				bHasItemUseBinding |= ModuleData.ModuleType == EMASkillModuleType::Item
					&& Binding.EventTag == UMASkillModuleItemAddon::GetUseEventTag();
			}
		}
		bHasItemAddon |= Addon->IsA<UMASkillModuleItemAddon>();
		AddonClasses.Add(Addon->GetClass());
	}
	if (ModuleData.ModuleType == EMASkillModuleType::Item
		&& !bHasItemAddon)
	{
		OutDiagnostic.Path = TEXT("Module.Addons");
		OutDiagnostic.Message = FText::FromString(TEXT("Item modules require an Item addon."));
		return false;
	}
	if (ModuleData.ModuleType == EMASkillModuleType::Item
		&& !bHasItemUseBinding)
	{
		OutDiagnostic.Path = TEXT("Module.Addons");
		OutDiagnostic.Message = FText::FromString(
			TEXT("Item modules require an Event.Item.Use binding."));
		return false;
	}
	return true;
}
