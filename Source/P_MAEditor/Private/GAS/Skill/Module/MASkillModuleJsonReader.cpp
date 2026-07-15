#include "GAS/Skill/Module/MASkillModuleJsonReader.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "GAS/Skill/Module/MASkillModuleJsonValidator.h"
#include "Internationalization/Text.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/TextProperty.h"
#include "UObject/UnrealType.h"

static const FString ClassNameField = TEXT("_ClassName");
static const FString ModuleIdField = TEXT("ModuleId");
static const FString ModuleField = TEXT("Module");
static const FString TextNamespace = TEXT("SkillModule");

static bool Fail(FText& OutError, const FString& Message)
{
	OutError = FText::FromString(Message);
	return false;
}

static bool ReadRoot(
	const TSharedRef<FJsonObject>& JsonObject,
	int32& OutModuleId,
	TSharedPtr<FJsonObject>& OutModuleObject,
	FText& OutError);
static void AssignLocalizationKeys(int32 ModuleId, FMASkillModuleData& ModuleData);
static bool AssignAddonsToOuter(FMASkillModuleData& ModuleData, UObject& Outer, FText& OutError);

bool FMASkillModuleJsonReader::Read(
	const FString& Json,
	UObject& AddonOuter,
	int32& OutModuleId,
	FMASkillModuleData& OutModuleData,
	FText& OutError)
{
	OutModuleId = 0;
	OutModuleData = FMASkillModuleData();
	OutError = FText::GetEmpty();

	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return Fail(OutError, FString::Printf(TEXT("Invalid JSON: %s"), *Reader->GetErrorMessage()));
	}

	int32 ModuleId = 0;
	TSharedPtr<FJsonObject> ModuleObject;
	if (!ReadRoot(JsonObject.ToSharedRef(), ModuleId, ModuleObject, OutError)) return false;

	FMASkillModuleData ModuleData;
	FText ImportError;
	if (!FJsonObjectConverter::JsonObjectToUStruct(
		ModuleObject.ToSharedRef(),
		FMASkillModuleData::StaticStruct(),
		&ModuleData,
		CPF_Edit,
		CPF_Transient | CPF_Deprecated | CPF_EditConst,
		false,
		&ImportError))
	{
		OutError = FText::Format(
			NSLOCTEXT("MASkillModuleJsonReader", "ImportFailed", "Failed to deserialize the module data.\n{0}"),
			ImportError);
		return false;
	}

	if (!FMASkillModuleJsonValidator::Validate(ModuleId, ModuleData, OutError)) return false;
	AssignLocalizationKeys(ModuleId, ModuleData);
	if (!AssignAddonsToOuter(ModuleData, AddonOuter, OutError)) return false;

	OutModuleId = ModuleId;
	OutModuleData = MoveTemp(ModuleData);
	return true;
}

static bool IsAuthoredProperty(const FProperty& Property)
{
	return Property.HasAnyPropertyFlags(CPF_Edit)
		&& !Property.HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated | CPF_EditConst);
}

static FString AppendPropertyPath(const FString& Path, const FString& PropertyName)
{
	return Path.IsEmpty() ? PropertyName : Path + TEXT(".") + PropertyName;
}

static FString AppendArrayPath(const FString& Path, const int32 Index)
{
	return FString::Printf(TEXT("%s[%d]"), *Path, Index);
}

static FProperty* FindAuthoredProperty(const UStruct& Struct, const FString& PropertyName)
{
	for (TFieldIterator<FProperty> It(&Struct); It; ++It)
	{
		FProperty* Property = *It;
		if (IsAuthoredProperty(*Property) && Struct.GetAuthoredNameForField(Property) == PropertyName)
		{
			return Property;
		}
	}
	return nullptr;
}

static bool ValidateJsonObject(
	const UStruct& Struct,
	const TSharedRef<FJsonObject>& JsonObject,
	const FString& Path,
	bool bAllowClassField,
	FText& OutError);

static bool ValidateJsonValue(
	FProperty& Property,
	const TSharedPtr<FJsonValue>& JsonValue,
	const FString& Path,
	FText& OutError)
{
	if (JsonValue->IsNull()) return true;

	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(&Property))
	{
		if (JsonValue->Type != EJson::Array)
		{
			return Fail(OutError, FString::Printf(TEXT("%s must be an array."), *Path));
		}

		const TArray<TSharedPtr<FJsonValue>>& Values = JsonValue->AsArray();
		for (int32 Index = 0; Index < Values.Num(); ++Index)
		{
			if (!ValidateJsonValue(
				*ArrayProperty->Inner,
				Values[Index],
				AppendArrayPath(Path, Index),
				OutError))
			{
				return false;
			}
		}
		return true;
	}

	if (CastField<FTextProperty>(&Property))
	{
		return JsonValue->Type == EJson::String
			|| Fail(OutError, FString::Printf(TEXT("%s must be a string."), *Path));
	}

	if (FStructProperty* StructProperty = CastField<FStructProperty>(&Property))
	{
		return JsonValue->Type != EJson::Object
			|| ValidateJsonObject(
				*StructProperty->Struct,
				JsonValue->AsObject().ToSharedRef(),
				Path,
				false,
				OutError);
	}

	FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(&Property);
	if (!ObjectProperty || JsonValue->Type != EJson::Object) return true;

	const TSharedRef<FJsonObject> Object = JsonValue->AsObject().ToSharedRef();
	FString ClassPath;
	if (!Object->TryGetStringField(ClassNameField, ClassPath))
	{
		return Fail(OutError, FString::Printf(TEXT("%s is missing _ClassName."), *Path));
	}

	UClass* ObjectClass = FSoftClassPath(ClassPath).TryLoadClass<UObject>();
	if (!ObjectClass)
	{
		return Fail(OutError, FString::Printf(TEXT("%s references unknown class '%s'."), *Path, *ClassPath));
	}
	if (!ObjectClass->IsChildOf(ObjectProperty->PropertyClass))
	{
		return Fail(OutError, FString::Printf(
			TEXT("%s class '%s' is not a %s."),
			*Path,
			*ClassPath,
			*ObjectProperty->PropertyClass->GetName()));
	}
	if (ObjectClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return Fail(OutError, FString::Printf(TEXT("%s class '%s' is abstract."), *Path, *ClassPath));
	}

	return ValidateJsonObject(*ObjectClass, Object, Path, true, OutError);
}

static bool ValidateJsonObject(
	const UStruct& Struct,
	const TSharedRef<FJsonObject>& JsonObject,
	const FString& Path,
	const bool bAllowClassField,
	FText& OutError)
{
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Field : JsonObject->Values)
	{
		if (Field.Key == ClassNameField)
		{
			if (bAllowClassField) continue;
			return Fail(OutError, FString::Printf(TEXT("%s contains an unexpected _ClassName."), *Path));
		}

		FProperty* Property = FindAuthoredProperty(Struct, Field.Key);
		const FString FieldPath = AppendPropertyPath(Path, Field.Key);
		if (!Property)
		{
			return Fail(OutError, FString::Printf(TEXT("%s is not a supported field."), *FieldPath));
		}
		if (!ValidateJsonValue(*Property, Field.Value, FieldPath, OutError)) return false;
	}
	return true;
}

static void AssignLocalizationKey(FText& Text, const int32 ModuleId, const TCHAR* FieldPath)
{
	if (Text.IsEmpty()) return;

	Text = FText::ChangeKey(
		TextNamespace,
		FString::Printf(TEXT("M_%d_%s"), ModuleId, FieldPath),
		Text);
}

static void AssignLocalizationKeys(const int32 ModuleId, FMASkillModuleData& ModuleData)
{
	AssignLocalizationKey(
		ModuleData.DisplayData.DisplayName,
		ModuleId,
		TEXT("DisplayData_DisplayName"));
	AssignLocalizationKey(
		ModuleData.DisplayData.Description,
		ModuleId,
		TEXT("DisplayData_Description"));
	AssignLocalizationKey(
		ModuleData.DisplayData.NameData.Keyword,
		ModuleId,
		TEXT("DisplayData_NameData_Keyword"));
}

static bool ReadRoot(
	const TSharedRef<FJsonObject>& JsonObject,
	int32& OutModuleId,
	TSharedPtr<FJsonObject>& OutModuleObject,
	FText& OutError)
{
	if (JsonObject->Values.Num() != 2
		|| !JsonObject->HasTypedField<EJson::Number>(ModuleIdField)
		|| !JsonObject->HasTypedField<EJson::Object>(ModuleField))
	{
		return Fail(OutError, TEXT("JSON must contain only ModuleId and Module."));
	}

	const double ModuleIdNumber = JsonObject->GetNumberField(ModuleIdField);
	if (ModuleIdNumber <= 0.0
		|| ModuleIdNumber > MAX_int32
		|| FMath::TruncToDouble(ModuleIdNumber) != ModuleIdNumber)
	{
		return Fail(OutError, TEXT("ModuleId must be a positive integer."));
	}

	TSharedPtr<FJsonObject> ModuleObject = JsonObject->GetObjectField(ModuleField);
	if (!ValidateJsonObject(
		*FMASkillModuleData::StaticStruct(),
		ModuleObject.ToSharedRef(),
		ModuleField,
		false,
		OutError))
	{
		return false;
	}

	OutModuleId = static_cast<int32>(ModuleIdNumber);
	OutModuleObject = MoveTemp(ModuleObject);
	return true;
}

static bool AssignAddonsToOuter(FMASkillModuleData& ModuleData, UObject& Outer, FText& OutError)
{
	for (UMASkillModuleAddon* Addon : ModuleData.Addons)
	{
		if (Addon->GetOuter() == &Outer) continue;

		const FName Name = MakeUniqueObjectName(&Outer, Addon->GetClass(), Addon->GetFName());
		if (!Addon->Rename(
			*Name.ToString(),
			&Outer,
			REN_DoNotDirty | REN_DontCreateRedirectors | REN_NonTransactional))
		{
			return Fail(OutError, FString::Printf(
				TEXT("Failed to assign addon '%s' to its owner."),
				*Addon->GetClass()->GetName()));
		}
	}
	return true;
}
