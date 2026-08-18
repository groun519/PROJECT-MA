#include "GAS/Skill/Module/Json/MASkillModuleJsonWriter.h"

#include "Dom/JsonObject.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "GAS/Skill/Module/Json/MASkillModuleDataValidator.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Internationalization/Text.h"
#include "JsonObjectConverter.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"

static const FString StructNameField = TEXT("_StructName");
static const FString StructValueField = TEXT("Value");

static TSharedPtr<FJsonValue> ExportModuleProperty(FProperty* Property, const void* Value);

static TSharedPtr<FJsonValue> ExportInstancedStruct(const FInstancedStruct& InstancedStruct)
{
	if (!InstancedStruct.IsValid()) return MakeShared<FJsonValueNull>();

	const UScriptStruct* ScriptStruct = InstancedStruct.GetScriptStruct();
	TSharedRef<FJsonObject> ValueObject = MakeShared<FJsonObject>();
	FJsonObjectConverter::CustomExportCallback ExportCallback;
	ExportCallback.BindStatic(&ExportModuleProperty);
	if (!FJsonObjectConverter::UStructToJsonObject(
		ScriptStruct,
		InstancedStruct.GetMemory(),
		ValueObject,
		CPF_Edit,
		CPF_Transient | CPF_Deprecated | CPF_EditConst,
		&ExportCallback,
		EJsonObjectConversionFlags::SkipStandardizeCase))
	{
		return MakeShared<FJsonValueNull>();
	}

	TSharedRef<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(StructNameField, ScriptStruct->GetPathName());
	Result->SetObjectField(StructValueField, ValueObject);
	return MakeShared<FJsonValueObject>(Result);
}

static TSharedPtr<FJsonValue> ExportModuleProperty(FProperty* Property, const void* Value)
{
	const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
	if (StructProperty && StructProperty->Struct == FGameplayTag::StaticStruct())
	{
		const FGameplayTag& Tag = *static_cast<const FGameplayTag*>(Value);
		return MakeShared<FJsonValueString>(Tag.IsValid() ? Tag.ToString() : FString());
	}
	if (StructProperty && StructProperty->Struct == FGameplayTagContainer::StaticStruct())
	{
		TArray<FString> TagNames;
		for (const FGameplayTag& Tag : *static_cast<const FGameplayTagContainer*>(Value))
		{
			TagNames.Add(Tag.ToString());
		}
		TagNames.Sort();

		TArray<TSharedPtr<FJsonValue>> Tags;
		Tags.Reserve(TagNames.Num());
		for (const FString& TagName : TagNames)
		{
			Tags.Add(MakeShared<FJsonValueString>(TagName));
		}
		return MakeShared<FJsonValueArray>(MoveTemp(Tags));
	}
	if (StructProperty && StructProperty->Struct == FInstancedStruct::StaticStruct())
	{
		return ExportInstancedStruct(*static_cast<const FInstancedStruct*>(Value));
	}

	if (const FTextProperty* TextProperty = CastField<FTextProperty>(Property))
	{
		const FText Text = TextProperty->GetPropertyValue(Value);
		const FString* Source = FTextInspector::GetSourceString(Text);
		return MakeShared<FJsonValueString>(Source ? *Source : Text.ToString());
	}

	const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property);
	if (ObjectProperty
		&& Property->HasAnyPropertyFlags(CPF_PersistentInstance)
		&& !ObjectProperty->GetObjectPropertyValue(Value))
	{
		return MakeShared<FJsonValueNull>();
	}

	return nullptr;
}

bool FMASkillModuleJsonWriter::Write(
	const int32 ModuleId,
	const FMASkillModuleData& ModuleData,
	FString& OutJson,
	FText& OutError)
{
	OutJson.Reset();
	OutError = FText::GetEmpty();
	FMASkillModuleDiagnostic Diagnostic;
	if (!FMASkillModuleDataValidator::Validate(ModuleId, ModuleData, Diagnostic))
	{
		OutError = Diagnostic.ToText();
		return false;
	}

	TSharedRef<FJsonObject> ModuleObject = MakeShared<FJsonObject>();
	FJsonObjectConverter::CustomExportCallback ExportCallback;
	ExportCallback.BindStatic(&ExportModuleProperty);
	if (!FJsonObjectConverter::UStructToJsonObject(
		FMASkillModuleData::StaticStruct(),
		&ModuleData,
		ModuleObject,
		CPF_Edit,
		CPF_Transient | CPF_Deprecated | CPF_EditConst,
		&ExportCallback,
		EJsonObjectConversionFlags::SkipStandardizeCase))
	{
		OutError = FText::FromString(TEXT("Failed to serialize the module data."));
		return false;
	}

	TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	JsonObject->SetNumberField(TEXT("ModuleId"), ModuleId);
	JsonObject->SetObjectField(TEXT("Module"), ModuleObject);
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutJson);
	if (!FJsonSerializer::Serialize(JsonObject, Writer))
	{
		OutError = FText::FromString(TEXT("Failed to write the module JSON."));
		return false;
	}
	Writer->Close();
	return true;
}
