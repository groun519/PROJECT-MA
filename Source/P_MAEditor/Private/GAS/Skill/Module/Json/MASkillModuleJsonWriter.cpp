#include "GAS/Skill/Module/Json/MASkillModuleJsonWriter.h"

#include "Dom/JsonObject.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "GAS/Skill/Module/Json/MASkillModuleDataValidator.h"
#include "Internationalization/Text.h"
#include "JsonObjectConverter.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/TextProperty.h"

static TSharedPtr<FJsonValue> ExportSourceText(FProperty* Property, const void* Value)
{
	const FTextProperty* TextProperty = CastField<FTextProperty>(Property);
	if (!TextProperty) return nullptr;

	const FText Text = TextProperty->GetPropertyValue(Value);
	const FString* Source = FTextInspector::GetSourceString(Text);
	return MakeShared<FJsonValueString>(Source ? *Source : Text.ToString());
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
	ExportCallback.BindStatic(&ExportSourceText);
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
