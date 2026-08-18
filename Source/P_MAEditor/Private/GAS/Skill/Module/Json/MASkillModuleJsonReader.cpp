#include "GAS/Skill/Module/Json/MASkillModuleJsonReader.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "GAS/Skill/Module/Json/MASkillModuleDataValidator.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonFile.h"
#include "GAS/Skill/Payload/MASkillPayloadStructBase.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "InstancedReferenceSubobjectHelper.h"
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
static const FString ModuleQualityField = TEXT("ModuleQuality");
static const FString ModuleTypeField = TEXT("ModuleType");
static const FString PayloadsField = TEXT("Payloads");
static const FString RarityField = TEXT("Rarity");
static const FString StructNameField = TEXT("_StructName");
static const FString StructValueField = TEXT("Value");
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
static bool PrepareJsonObject(
	const UStruct& Struct,
	TSharedRef<FJsonObject> JsonObject,
	const FString& Path,
	bool bAllowClassField,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics);
static void AssignLocalizationKeys(int32 ModuleId, FMASkillModuleData& ModuleData);
static bool ImportPayloadStructs(
	const TSharedRef<FJsonObject>& ModuleObject,
	FMASkillModuleData& ModuleData,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics);
static bool AssignInstancedObjectsToOuter(
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
	FMASkillModuleJsonHeader& OutHeader,
	FText& OutError)
{
	OutHeader = FMASkillModuleJsonHeader();
	OutError = FText::GetEmpty();

	TArray<FMASkillModuleDiagnostic> Diagnostics;
	TSharedPtr<FJsonObject> ModuleObject;
	if (!ReadRoot(Json, OutHeader.ModuleId, ModuleObject, Diagnostics))
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
		OutHeader.ModuleName = FName(*ModuleName);
	}

	if (ModuleObject->HasField(ModuleTypeField))
	{
		FString ModuleTypeName;
		if (!ModuleObject->TryGetStringField(ModuleTypeField, ModuleTypeName))
		{
			Fail(Diagnostics, TEXT("Must be a string."), TEXT("Module.ModuleType"));
			OutError = Diagnostics[0].ToText();
			return false;
		}

		const int64 ModuleTypeValue = StaticEnum<EMASkillModuleType>()->GetValueByNameString(ModuleTypeName);
		OutHeader.ModuleType = static_cast<EMASkillModuleType>(ModuleTypeValue);
		if (OutHeader.ModuleType != EMASkillModuleType::Module
			&& OutHeader.ModuleType != EMASkillModuleType::Item
			&& OutHeader.ModuleType != EMASkillModuleType::Sub)
		{
			Fail(Diagnostics, TEXT("Unknown skill module type."), TEXT("Module.ModuleType"));
			OutError = Diagnostics[0].ToText();
			return false;
		}
	}

	if (ModuleObject->HasField(ModuleQualityField)
		&& !ModuleObject->HasTypedField<EJson::Object>(ModuleQualityField))
	{
		Fail(Diagnostics, TEXT("Must be an object."), TEXT("Module.ModuleQuality"));
		OutError = Diagnostics[0].ToText();
		return false;
	}

	const TSharedPtr<FJsonObject>* QualityObject = nullptr;
	if (ModuleObject->TryGetObjectField(ModuleQualityField, QualityObject)
		&& (*QualityObject)->HasField(RarityField))
	{
		FString RarityName;
		if (!(*QualityObject)->TryGetStringField(RarityField, RarityName))
		{
			Fail(Diagnostics, TEXT("Must be a string."), TEXT("Module.ModuleQuality.Rarity"));
			OutError = Diagnostics[0].ToText();
			return false;
		}

		const int64 RarityValue = StaticEnum<EMAModuleRarity>()->GetValueByNameString(RarityName);
		if (RarityValue == INDEX_NONE)
		{
			Fail(Diagnostics, TEXT("Unknown module rarity."), TEXT("Module.ModuleQuality.Rarity"));
			OutError = Diagnostics[0].ToText();
			return false;
		}
		OutHeader.ModuleRarity = static_cast<EMAModuleRarity>(RarityValue);
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
	if (!PrepareJsonObject(
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
	if (!ImportPayloadStructs(ModuleObject.ToSharedRef(), ModuleData, Result.Diagnostics))
	{
		return Result;
	}

	FMASkillModuleDiagnostic Diagnostic;
	if (!FMASkillModuleDataValidator::Validate(ModuleId, ModuleData, Diagnostic))
	{
		Result.Diagnostics.Add(MoveTemp(Diagnostic));
		return Result;
	}
	AssignLocalizationKeys(ModuleId, ModuleData);
	if (!AssignInstancedObjectsToOuter(ModuleData, AddonOuter, Result.Diagnostics)) return Result;

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

static UScriptStruct* ResolvePayloadStruct(
	const TSharedRef<FJsonObject>& JsonObject,
	const FString& Path,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	if (JsonObject->Values.Num() != 2
		|| !JsonObject->HasTypedField<EJson::String>(StructNameField)
		|| !JsonObject->HasTypedField<EJson::Object>(StructValueField))
	{
		Fail(
			OutDiagnostics,
			TEXT("Must contain only _StructName and Value."),
			Path);
		return nullptr;
	}

	const FString StructPath = JsonObject->GetStringField(StructNameField);
	UScriptStruct* ScriptStruct = LoadObject<UScriptStruct>(nullptr, *StructPath);
	if (!ScriptStruct)
	{
		Fail(
			OutDiagnostics,
			FString::Printf(TEXT("References unknown struct '%s'."), *StructPath),
			AppendPropertyPath(Path, StructNameField));
		return nullptr;
	}
	if (!ScriptStruct->IsChildOf(FMASkillPayloadStructBase::StaticStruct()))
	{
		Fail(
			OutDiagnostics,
			FString::Printf(
				TEXT("Struct '%s' is not a skill payload struct."),
				*StructPath),
			AppendPropertyPath(Path, StructNameField));
		return nullptr;
	}
	return ScriptStruct;
}

static bool ResolveGameplayTag(
	const FString& TagName,
	FGameplayTag& OutTag,
	const FString& Path,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	OutTag = FGameplayTag();
	if (TagName.IsEmpty()) return true;

	OutTag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);
	return OutTag.IsValid()
		|| Fail(
			OutDiagnostics,
			FString::Printf(TEXT("References unknown gameplay tag '%s'."), *TagName),
			Path);
}

static bool PrepareJsonValue(
	FProperty& Property,
	TSharedPtr<FJsonValue>& JsonValue,
	const FString& Path,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(&Property))
	{
		if (JsonValue->Type != EJson::Array)
		{
			return Fail(OutDiagnostics, TEXT("Must be an array."), Path);
		}

		TArray<TSharedPtr<FJsonValue>> Values = JsonValue->AsArray();
		for (int32 Index = 0; Index < Values.Num(); ++Index)
		{
			if (!PrepareJsonValue(
				*ArrayProperty->Inner,
				Values[Index],
				AppendArrayPath(Path, Index),
				OutDiagnostics))
			{
				return false;
			}
		}
		JsonValue = MakeShared<FJsonValueArray>(MoveTemp(Values));
		return true;
	}

	if (FStructProperty* StructProperty = CastField<FStructProperty>(&Property))
	{
		if (StructProperty->Struct == FGameplayTag::StaticStruct())
		{
			if (JsonValue->Type != EJson::String)
			{
				return Fail(OutDiagnostics, TEXT("Must be a gameplay tag string."), Path);
			}

			FGameplayTag Tag;
			return ResolveGameplayTag(JsonValue->AsString(), Tag, Path, OutDiagnostics);
		}
		if (StructProperty->Struct == FGameplayTagContainer::StaticStruct())
		{
			if (JsonValue->Type != EJson::Array)
			{
				return Fail(OutDiagnostics, TEXT("Must be an array of gameplay tag strings."), Path);
			}

			FGameplayTagContainer Tags;
			TSet<FGameplayTag> UniqueTags;
			const TArray<TSharedPtr<FJsonValue>>& Values = JsonValue->AsArray();
			for (int32 Index = 0; Index < Values.Num(); ++Index)
			{
				const FString TagPath = AppendArrayPath(Path, Index);
				if (Values[Index]->Type != EJson::String)
				{
					return Fail(OutDiagnostics, TEXT("Must be a gameplay tag string."), TagPath);
				}

				FGameplayTag Tag;
				if (!ResolveGameplayTag(Values[Index]->AsString(), Tag, TagPath, OutDiagnostics)) return false;
				if (!Tag.IsValid())
				{
					return Fail(OutDiagnostics, TEXT("Must not be empty."), TagPath);
				}
				if (UniqueTags.Contains(Tag))
				{
					return Fail(OutDiagnostics, TEXT("Duplicates a gameplay tag."), TagPath);
				}

				UniqueTags.Add(Tag);
				Tags.AddTag(Tag);
			}
			JsonValue = MakeShared<FJsonValueString>(Tags.ToString());
			return true;
		}
	}

	if (JsonValue->IsNull()) return true;

	if (CastField<FTextProperty>(&Property))
	{
		return JsonValue->Type == EJson::String
			|| Fail(OutDiagnostics, TEXT("Must be a string."), Path);
	}

	if (FStructProperty* StructProperty = CastField<FStructProperty>(&Property))
	{
		if (StructProperty->Struct == FInstancedStruct::StaticStruct())
		{
			if (JsonValue->Type != EJson::Object)
			{
				return Fail(OutDiagnostics, TEXT("Must be an object."), Path);
			}

			const TSharedRef<FJsonObject> Object = JsonValue->AsObject().ToSharedRef();
			UScriptStruct* ScriptStruct = ResolvePayloadStruct(Object, Path, OutDiagnostics);
			return ScriptStruct
				&& PrepareJsonObject(
					*ScriptStruct,
					Object->GetObjectField(StructValueField).ToSharedRef(),
					AppendPropertyPath(Path, StructValueField),
					false,
					OutDiagnostics);
		}

		return JsonValue->Type != EJson::Object
			|| PrepareJsonObject(
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

	return PrepareJsonObject(*ObjectClass, Object, Path, true, OutDiagnostics);
}

static bool PrepareJsonObject(
	const UStruct& Struct,
	TSharedRef<FJsonObject> JsonObject,
	const FString& Path,
	const bool bAllowClassField,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	for (auto& Field : JsonObject->Values)
	{
		const FString FieldName(Field.Key.ToView());
		if (FieldName == ClassNameField)
		{
			if (bAllowClassField) continue;
			return Fail(OutDiagnostics, TEXT("Contains an unexpected _ClassName."), Path);
		}

		FProperty* Property = FindAuthoredProperty(Struct, FieldName);
		const FString FieldPath = AppendPropertyPath(Path, FieldName);
		if (!Property)
		{
			return Fail(OutDiagnostics, TEXT("Is not a supported field."), FieldPath);
		}
		if (!PrepareJsonValue(*Property, Field.Value, FieldPath, OutDiagnostics)) return false;
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

static bool ImportPayloadStructs(
	const TSharedRef<FJsonObject>& ModuleObject,
	FMASkillModuleData& ModuleData,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	const TArray<TSharedPtr<FJsonValue>>* PayloadValues = nullptr;
	if (!ModuleObject->TryGetArrayField(PayloadsField, PayloadValues)) return true;
	if (PayloadValues->Num() != ModuleData.Payloads.Num())
	{
		return Fail(OutDiagnostics, TEXT("Failed to match imported payload entries."), TEXT("Module.Payloads"));
	}

	const FProperty* Property = FindFProperty<FProperty>(
		FMASkillPayloadEntry::StaticStruct(),
		GET_MEMBER_NAME_CHECKED(FMASkillPayloadEntry, StructValue));
	check(Property);
	const FString AuthoredStructValueField =
		FMASkillPayloadEntry::StaticStruct()->GetAuthoredNameForField(Property);

	for (int32 Index = 0; Index < PayloadValues->Num(); ++Index)
	{
		const TSharedPtr<FJsonObject> PayloadObject = (*PayloadValues)[Index]->AsObject();
		const TSharedPtr<FJsonValue> StructJsonValue =
			PayloadObject->TryGetField(AuthoredStructValueField);
		if (!StructJsonValue.IsValid() || StructJsonValue->IsNull()) continue;

		const FString Path = AppendPropertyPath(
			AppendArrayPath(TEXT("Module.Payloads"), Index),
			AuthoredStructValueField);
		const TSharedRef<FJsonObject> StructObject = StructJsonValue->AsObject().ToSharedRef();
		UScriptStruct* ScriptStruct = ResolvePayloadStruct(StructObject, Path, OutDiagnostics);
		if (!ScriptStruct) return false;

		FInstancedStruct& InstancedStruct = ModuleData.Payloads[Index].StructValue;
		InstancedStruct.InitializeAs(ScriptStruct);
		FText ImportError;
		if (!FJsonObjectConverter::JsonObjectToUStruct(
			StructObject->GetObjectField(StructValueField).ToSharedRef(),
			ScriptStruct,
			InstancedStruct.GetMutableMemory(),
			CPF_Edit,
			CPF_Transient | CPF_Deprecated | CPF_EditConst,
			false,
			&ImportError))
		{
			return Fail(
				OutDiagnostics,
				FString::Printf(TEXT("Failed to deserialize payload struct: %s"), *ImportError.ToString()),
				Path);
		}
	}
	return true;
}

static bool AssignStructObjectsToOuter(
	const UStruct& Struct,
	void* StructMemory,
	UObject& Outer,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	bool bSucceeded = true;
	for (FProperty* Property = Struct.RefLink; Property && bSucceeded; Property = Property->NextRef)
	{
		for (int32 ArrayIndex = 0; ArrayIndex < Property->ArrayDim && bSucceeded; ++ArrayIndex)
		{
			FInstancedPropertyPath PropertyPath(Property, ArrayIndex);
			void* Value = Property->ContainerPtrToValuePtr<void>(StructMemory, ArrayIndex);
			FFindInstancedReferenceSubobjectHelper::ForEachInstancedSubObject<void*>(
				PropertyPath,
				Value,
				[&](const FInstancedSubObjRef& Ref, void*)
			{
				UObject* Object = Ref.SubObjInstance;
				if (!Object || Object->IsIn(&Outer)) return;

				const FName Name = MakeUniqueObjectName(&Outer, Object->GetClass(), Object->GetFName());
				if (!Object->Rename(
					*Name.ToString(),
					&Outer,
					REN_DoNotDirty | REN_DontCreateRedirectors | REN_NonTransactional))
				{
					Fail(
						OutDiagnostics,
						FString::Printf(
							TEXT("Failed to assign '%s' to its module owner."),
							*Object->GetClass()->GetName()),
						TEXT("Module"));
					bSucceeded = false;
				}
			});
		}
	}
	return bSucceeded;
}

static bool AssignInstancedObjectsToOuter(
	FMASkillModuleData& ModuleData,
	UObject& Outer,
	TArray<FMASkillModuleDiagnostic>& OutDiagnostics)
{
	if (!AssignStructObjectsToOuter(
		*FMASkillModuleData::StaticStruct(),
		&ModuleData,
		Outer,
		OutDiagnostics))
	{
		return false;
	}

	// FInstancedStruct hides its dynamic property graph from static reflection.
	for (FMASkillPayloadEntry& Payload : ModuleData.Payloads)
	{
		FInstancedStruct& StructValue = Payload.StructValue;
		if (StructValue.IsValid()
			&& !AssignStructObjectsToOuter(
				*StructValue.GetScriptStruct(),
				StructValue.GetMutableMemory(),
				Outer,
				OutDiagnostics))
		{
			return false;
		}
	}
	return true;
}
