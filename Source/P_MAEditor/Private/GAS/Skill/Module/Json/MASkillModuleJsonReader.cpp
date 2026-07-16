#include "GAS/Skill/Module/Json/MASkillModuleJsonReader.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "GAS/Skill/Module/Json/MASkillModuleDataValidator.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonFile.h"
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
static const FString ModuleNameField = TEXT("ModuleName");
static const FString TextNamespace = TEXT("SkillModule");

static bool Fail(
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics,
	const FString& Message,
	const FString& Path = FString())
{
	FMASkillModuleDiagnostic& Diagnostic = OutDiagnostics.AddDefaulted_GetRef();
	Diagnostic.Path = Path;
	Diagnostic.Message = FText::FromString(Message);
	return false;
}

static bool ReadRoot(
	const FString& Json,
	int32& OutModuleId,
	TSharedPtr<FJsonObject>& OutModuleObject,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics);
static bool ValidateJsonObject(
	const UStruct& Struct,
	const TSharedRef<FJsonObject>& JsonObject,
	const FString& Path,
	bool bAllowClassField,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics);
static void AssignLocalizationKeys(int32 ModuleId, FMASkillModuleData& ModuleData);
static bool AssignAddonsToOuter(
	FMASkillModuleData& ModuleData,
	UObject& Outer,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics);
static FMASkillModuleReadResult ReadJson(
	const FString& Json,
	UObject& AddonOuter,
	int32 ExpectedModuleId);

bool FMASkillModuleReadResult::IsValid() const
{
	return ModuleId > 0
		&& !Diagnostics.ContainsByPredicate([](const FMASkillModuleDiagnostic& Diagnostic)
	{
		return Diagnostic.Severity == EMASkillModuleDiagnosticSeverity::Error;
	});
}

FText FMASkillModuleReadResult::GetDiagnosticsText() const
{
	TArray<FString> Messages;
	Messages.Reserve(Diagnostics.Num());
	for (const FMASkillModuleDiagnostic& Diagnostic : Diagnostics)
	{
		Messages.Add(Diagnostic.ToText().ToString());
	}
	return FText::FromString(FString::Join(Messages, TEXT("\n")));
}

bool FMASkillModuleJsonReader::ReadHeader(
	const FString& Json,
	int32& OutModuleId,
	FName& OutModuleName,
	FText& OutError)
{
	OutModuleId = 0;
	OutModuleName = NAME_None;
	OutError = FText::GetEmpty();

	TArray<FMASkillModuleDiagnostic> Diagnostics;
	TSharedPtr<FJsonObject> ModuleObject;
	if (!ReadRoot(Json, OutModuleId, ModuleObject, Diagnostics))
	{
		OutError = Diagnostics[0].ToText();
		return false;
	}
	if (ModuleObject->HasField(ModuleNameField)
		&& !ModuleObject->HasTypedField<EJson::String>(ModuleNameField))
	{
		Fail(Diagnostics, TEXT("Must be a string."), TEXT("Module.ModuleName"));
		OutError = Diagnostics[0].ToText();
		return false;
	}

	FString ModuleName;
	if (ModuleObject->TryGetStringField(ModuleNameField, ModuleName))
	{
		OutModuleName = FName(*ModuleName);
	}
	return true;
}

FMASkillModuleReadResult FMASkillModuleJsonReader::Read(const FString& Json, UObject& AddonOuter)
{
	return ReadJson(Json, AddonOuter, 0);
}

FMASkillModuleReadResult FMASkillModuleJsonReader::Read(
	const FMASkillModuleJsonSource& Source,
	UObject& AddonOuter)
{
	if (Source.ModuleId <= 0)
	{
		FMASkillModuleReadResult Result;
		Fail(Result.Diagnostics, TEXT("Source does not have a valid file ModuleId."));
		return Result;
	}
	return ReadJson(Source.ToJson(), AddonOuter, Source.ModuleId);
}

static FMASkillModuleReadResult ReadJson(
	const FString& Json,
	UObject& AddonOuter,
	const int32 ExpectedModuleId)
{
	FMASkillModuleReadResult Result;

	int32 ModuleId = 0;
	TSharedPtr<FJsonObject> ModuleObject;
	if (!ReadRoot(Json, ModuleId, ModuleObject, Result.Diagnostics)) return Result;
	if (ExpectedModuleId > 0 && ModuleId != ExpectedModuleId)
	{
		Fail(
			Result.Diagnostics,
			FString::Printf(TEXT("Must match source file ModuleId %d."), ExpectedModuleId),
			TEXT("ModuleId"));
		return Result;
	}
	if (!ValidateJsonObject(
		*FMASkillModuleData::StaticStruct(),
		ModuleObject.ToSharedRef(),
		ModuleField,
		false,
		Result.Diagnostics))
	{
		return Result;
	}

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
		Fail(
			Result.Diagnostics,
			FText::Format(
				NSLOCTEXT("MASkillModuleJsonReader", "ImportFailed", "Failed to deserialize the module data.\n{0}"),
				ImportError).ToString());
		return Result;
	}

	FMASkillModuleDiagnostic Diagnostic;
	if (!FMASkillModuleDataValidator::Validate(ModuleId, ModuleData, Diagnostic))
	{
		Result.Diagnostics.Add(MoveTemp(Diagnostic));
		return Result;
	}
	AssignLocalizationKeys(ModuleId, ModuleData);
	if (!AssignAddonsToOuter(ModuleData, AddonOuter, Result.Diagnostics)) return Result;

	Result.ModuleId = ModuleId;
	Result.ModuleData = MoveTemp(ModuleData);
	return Result;
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

static bool ValidateJsonValue(
	FProperty& Property,
	const TSharedPtr<FJsonValue>& JsonValue,
	const FString& Path,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	if (JsonValue->IsNull()) return true;

	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(&Property))
	{
		if (JsonValue->Type != EJson::Array)
		{
			return Fail(OutDiagnostics, TEXT("Must be an array."), Path);
		}

		const TArray<TSharedPtr<FJsonValue>>& Values = JsonValue->AsArray();
		for (int32 Index = 0; Index < Values.Num(); ++Index)
		{
			if (!ValidateJsonValue(
				*ArrayProperty->Inner,
				Values[Index],
				AppendArrayPath(Path, Index),
				OutDiagnostics))
			{
				return false;
			}
		}
		return true;
	}

	if (CastField<FTextProperty>(&Property))
	{
		return JsonValue->Type == EJson::String
			|| Fail(OutDiagnostics, TEXT("Must be a string."), Path);
	}

	if (FStructProperty* StructProperty = CastField<FStructProperty>(&Property))
	{
		return JsonValue->Type != EJson::Object
			|| ValidateJsonObject(
				*StructProperty->Struct,
				JsonValue->AsObject().ToSharedRef(),
				Path,
				false,
				OutDiagnostics);
	}

	FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(&Property);
	if (!ObjectProperty) return true;

	const bool bPersistentInstance = Property.HasAnyPropertyFlags(CPF_PersistentInstance);
	if (!bPersistentInstance)
	{
		return JsonValue->Type == EJson::String
			|| Fail(OutDiagnostics, TEXT("Must be an object reference string."), Path);
	}
	if (JsonValue->Type != EJson::Object)
	{
		return Fail(OutDiagnostics, TEXT("Must be an object."), Path);
	}

	const TSharedRef<FJsonObject> Object = JsonValue->AsObject().ToSharedRef();
	FString ClassPath;
	if (!Object->TryGetStringField(ClassNameField, ClassPath))
	{
		return Fail(OutDiagnostics, TEXT("Is missing _ClassName."), Path);
	}

	UClass* ObjectClass = FSoftClassPath(ClassPath).TryLoadClass<UObject>();
	if (!ObjectClass)
	{
		return Fail(
			OutDiagnostics,
			FString::Printf(TEXT("References unknown class '%s'."), *ClassPath),
			Path);
	}
	if (!ObjectClass->IsChildOf(ObjectProperty->PropertyClass))
	{
		return Fail(OutDiagnostics, FString::Printf(
			TEXT("Class '%s' is not a %s."),
			*ClassPath,
			*ObjectProperty->PropertyClass->GetName()),
			Path);
	}
	if (ObjectClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return Fail(
			OutDiagnostics,
			FString::Printf(TEXT("Class '%s' is abstract."), *ClassPath),
			Path);
	}

	return ValidateJsonObject(*ObjectClass, Object, Path, true, OutDiagnostics);
}

static bool ValidateJsonObject(
	const UStruct& Struct,
	const TSharedRef<FJsonObject>& JsonObject,
	const FString& Path,
	const bool bAllowClassField,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Field : JsonObject->Values)
	{
		if (Field.Key == ClassNameField)
		{
			if (bAllowClassField) continue;
			return Fail(OutDiagnostics, TEXT("Contains an unexpected _ClassName."), Path);
		}

		FProperty* Property = FindAuthoredProperty(Struct, Field.Key);
		const FString FieldPath = AppendPropertyPath(Path, Field.Key);
		if (!Property)
		{
			return Fail(OutDiagnostics, TEXT("Is not a supported field."), FieldPath);
		}
		if (!ValidateJsonValue(*Property, Field.Value, FieldPath, OutDiagnostics)) return false;
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
	const FString& Json,
	int32& OutModuleId,
	TSharedPtr<FJsonObject>& OutModuleObject,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	TSharedPtr<FJsonObject> JsonObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		return Fail(OutDiagnostics, FString::Printf(TEXT("Invalid JSON: %s"), *Reader->GetErrorMessage()));
	}
	if (JsonObject->Values.Num() != 2
		|| !JsonObject->HasTypedField<EJson::Number>(ModuleIdField)
		|| !JsonObject->HasTypedField<EJson::Object>(ModuleField))
	{
		return Fail(OutDiagnostics, TEXT("JSON must contain only ModuleId and Module."));
	}

	const double ModuleIdNumber = JsonObject->GetNumberField(ModuleIdField);
	if (ModuleIdNumber <= 0.0
		|| ModuleIdNumber > MAX_int32
		|| FMath::TruncToDouble(ModuleIdNumber) != ModuleIdNumber)
	{
		return Fail(OutDiagnostics, TEXT("Must be a positive integer."), TEXT("ModuleId"));
	}

	OutModuleId = static_cast<int32>(ModuleIdNumber);
	OutModuleObject = JsonObject->GetObjectField(ModuleField);
	return true;
}

static bool AssignAddonsToOuter(
	FMASkillModuleData& ModuleData,
	UObject& Outer,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	for (int32 Index = 0; Index < ModuleData.Addons.Num(); ++Index)
	{
		UMASkillModuleAddon* Addon = ModuleData.Addons[Index];
		if (Addon->GetOuter() == &Outer) continue;

		const FName Name = MakeUniqueObjectName(&Outer, Addon->GetClass(), Addon->GetFName());
		if (!Addon->Rename(
			*Name.ToString(),
			&Outer,
			REN_DoNotDirty | REN_DontCreateRedirectors | REN_NonTransactional))
		{
			return Fail(
				OutDiagnostics,
				FString::Printf(
					TEXT("Failed to assign addon '%s' to its owner."),
					*Addon->GetClass()->GetName()),
				FString::Printf(TEXT("Module.Addons[%d]"), Index));
		}
	}
	return true;
}
