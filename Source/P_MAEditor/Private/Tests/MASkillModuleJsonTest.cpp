#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GAS/Skill/Addon/Effect/MASkillModuleGameplayEffectAddon.h"
#include "GAS/Skill/Addon/Sequence/MASkillModuleSequenceAddon.h"
#include "GAS/Skill/Addon/Stack/MASkillModuleStackAddon.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "GAS/Skill/Module/MASkillModuleJsonReader.h"
#include "GAS/Skill/Module/MASkillModuleJsonWriter.h"
#include "GAS/Skill/Sequence/Variants/MASkillSequenceModifier_Delay.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"

namespace
{
	template<typename PropertyType>
	PropertyType* FindPropertyChecked(const UStruct& OwnerStruct, const FName PropertyName)
	{
		PropertyType* Property = FindFProperty<PropertyType>(&OwnerStruct, PropertyName);
		check(Property);
		return Property;
	}

	template<typename ValueType, typename PropertyType>
	void SetProperty(UObject& Object, const FName PropertyName, const ValueType& Value)
	{
		PropertyType* Property = FindPropertyChecked<PropertyType>(*Object.GetClass(), PropertyName);
		Property->SetPropertyValue_InContainer(&Object, Value);
	}

	void AddObjectToArray(UObject& Owner, const FName PropertyName, UObject& Value)
	{
		FArrayProperty* ArrayProperty = FindPropertyChecked<FArrayProperty>(*Owner.GetClass(), PropertyName);
		FObjectPropertyBase* ObjectProperty = CastFieldChecked<FObjectPropertyBase>(ArrayProperty->Inner);
		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(&Owner));
		ObjectProperty->SetObjectPropertyValue(ArrayHelper.GetRawPtr(ArrayHelper.AddValue()), &Value);
	}

	template<typename StructType>
	StructType& AddStructToArray(UObject& Owner, const FName PropertyName)
	{
		FArrayProperty* ArrayProperty = FindPropertyChecked<FArrayProperty>(*Owner.GetClass(), PropertyName);
		FStructProperty* StructProperty = CastFieldChecked<FStructProperty>(ArrayProperty->Inner);
		check(StructProperty->Struct == StructType::StaticStruct());

		FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(&Owner));
		return *reinterpret_cast<StructType*>(ArrayHelper.GetRawPtr(ArrayHelper.AddValue()));
	}

	template<typename StructType>
	const StructType* GetFirstStruct(const UObject& Owner, const FName PropertyName)
	{
		FArrayProperty* ArrayProperty = FindPropertyChecked<FArrayProperty>(*Owner.GetClass(), PropertyName);
		FScriptArrayHelper ArrayHelper(
			ArrayProperty,
			ArrayProperty->ContainerPtrToValuePtr<void>(const_cast<UObject*>(&Owner)));
		return ArrayHelper.Num() > 0
			? reinterpret_cast<const StructType*>(ArrayHelper.GetRawPtr(0))
			: nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMASkillModuleJsonRoundTripTest,
	"P_MA.Skill.ModuleJson.RoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMASkillModuleJsonRoundTripTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UDataTable> SourceOwner(NewObject<UDataTable>());
	FMASkillModuleData Module;
	Module.ModuleName = TEXT("JsonRoundTrip");
	Module.DisplayData.DisplayName = FText::FromString(TEXT("Slash"));
	Module.DisplayData.Description = FText::FromString(TEXT("Round-trip test module"));
	Module.DisplayData.NameData.Keyword = FText::FromString(TEXT("Attack"));
	Module.DisplayData.IconData.Icon = LoadObject<UTexture2D>(
		nullptr,
		TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));

	UMASkillModuleStackAddon* StackAddon = NewObject<UMASkillModuleStackAddon>(SourceOwner.Get());
	SetProperty<int32, FIntProperty>(*StackAddon, TEXT("InitialStack"), 7);
	Module.Addons.Add(StackAddon);

	UMASkillModuleSequenceAddon* SequenceAddon = NewObject<UMASkillModuleSequenceAddon>(SourceOwner.Get());
	UMASkillSequenceModifier_Delay* DelayModifier = NewObject<UMASkillSequenceModifier_Delay>(SequenceAddon);
	SetProperty<float, FFloatProperty>(*DelayModifier, TEXT("TimeLimitSeconds"), 1.25f);
	SetProperty<bool, FBoolProperty>(*DelayModifier, TEXT("bShowProgress"), true);
	SetProperty<FText, FTextProperty>(*DelayModifier, TEXT("ProgressLabel"), FText::FromString(TEXT("Windup")));
	AddObjectToArray(*SequenceAddon, TEXT("SequenceModifiers"), *DelayModifier);
	Module.Addons.Add(SequenceAddon);

	UMASkillModuleGameplayEffectAddon* EffectAddon = NewObject<UMASkillModuleGameplayEffectAddon>(SourceOwner.Get());
	FMASkillModuleGameplayEffectConfig& EffectConfig =
		AddStructToArray<FMASkillModuleGameplayEffectConfig>(*EffectAddon, TEXT("GameplayEffects"));
	EffectConfig.BaseMagnitude = 13.f;
	FPropertyChangedEvent EffectPropertyChanged(
		FindPropertyChecked<FArrayProperty>(*EffectAddon->GetClass(), TEXT("GameplayEffects")));
	EffectAddon->PostEditChangeProperty(EffectPropertyChanged);
	TestNotNull(TEXT("Generated EffectDefinition exists before export"), EffectConfig.GetEffectDefinition());
	Module.Addons.Add(EffectAddon);

	FMASkillPayloadEntry& Payload = Module.Payloads.AddDefaulted_GetRef();
	Payload.ValueType = EMASkillPayloadValueType::Struct;
	Payload.StructValue.InitializeAs<FMASkillDamageConfig>();
	Payload.StructValue.GetMutable<FMASkillDamageConfig>().BaseDamage = 42.f;

	FString JsonA;
	FText Error;
	if (!TestTrue(TEXT("Serialize module data"), FMASkillModuleJsonWriter::Write(1201, Module, JsonA, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestTrue(TEXT("JSON contains polymorphic class data"), JsonA.Contains(TEXT("_ClassName")));
	TestTrue(TEXT("JSON contains plain text"), JsonA.Contains(TEXT("\"DisplayName\": \"Slash\"")));
	TestFalse(TEXT("JSON excludes text identity fields"), JsonA.Contains(TEXT("\"Key\"")));
	TestFalse(TEXT("Generated EffectDefinition is excluded"), JsonA.Contains(TEXT("EffectDefinition")));

	TStrongObjectPtr<UDataTable> LoadedOwner(NewObject<UDataTable>());
	int32 LoadedModuleId = 0;
	FMASkillModuleData LoadedModule;
	if (!TestTrue(
		TEXT("Deserialize module data"),
		FMASkillModuleJsonReader::Read(JsonA, *LoadedOwner, LoadedModuleId, LoadedModule, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("ModuleId round-trips"), LoadedModuleId, 1201);
	TestEqual(TEXT("ModuleName round-trips"), LoadedModule.ModuleName, FName(TEXT("JsonRoundTrip")));
	TestEqual(TEXT("DisplayName round-trips"), LoadedModule.DisplayData.DisplayName.ToString(), FString(TEXT("Slash")));
	TestEqual(
		TEXT("DisplayName namespace is stable"),
		FTextInspector::GetNamespace(LoadedModule.DisplayData.DisplayName).Get(FString()),
		FString(TEXT("SkillModule")));
	TestEqual(
		TEXT("DisplayName key is deterministic"),
		FTextInspector::GetKey(LoadedModule.DisplayData.DisplayName).Get(FString()),
		FString(TEXT("M_1201_DisplayData_DisplayName")));
	TestEqual(
		TEXT("Description key is deterministic"),
		FTextInspector::GetKey(LoadedModule.DisplayData.Description).Get(FString()),
		FString(TEXT("M_1201_DisplayData_Description")));
	TestEqual(
		TEXT("Keyword key is deterministic"),
		FTextInspector::GetKey(LoadedModule.DisplayData.NameData.Keyword).Get(FString()),
		FString(TEXT("M_1201_DisplayData_NameData_Keyword")));
	TestTrue(TEXT("External asset reference round-trips"), LoadedModule.DisplayData.IconData.Icon != nullptr);
	TestEqual(TEXT("Addon count round-trips"), LoadedModule.Addons.Num(), 3);
	TestTrue(TEXT("StackAddon class round-trips"), LoadedModule.Addons[0]->IsA<UMASkillModuleStackAddon>());
	TestTrue(TEXT("Loaded addons use the requested owner"), LoadedModule.Addons[0]->IsIn(LoadedOwner.Get()));

	const UMASkillModuleSequenceAddon* LoadedSequence =
		Cast<UMASkillModuleSequenceAddon>(LoadedModule.Addons[1]);
	TestNotNull(TEXT("SequenceAddon class round-trips"), LoadedSequence);
	if (LoadedSequence)
	{
		TestEqual(TEXT("Nested modifier count round-trips"), LoadedSequence->GetSequenceModifiers().Num(), 1);
		const UMASkillSequenceModifier_Delay* LoadedDelay =
			LoadedSequence->GetSequenceModifiers().Num() > 0
				? Cast<UMASkillSequenceModifier_Delay>(LoadedSequence->GetSequenceModifiers()[0])
				: nullptr;
		TestNotNull(TEXT("Nested modifier class round-trips"), LoadedDelay);
		if (LoadedDelay)
		{
			TestTrue(TEXT("Nested modifier keeps its addon owner"), LoadedDelay->GetOuter() == LoadedSequence);
			const FTextProperty* LabelProperty =
				FindPropertyChecked<FTextProperty>(*LoadedDelay->GetClass(), TEXT("ProgressLabel"));
			const FText Label = LabelProperty->GetPropertyValue_InContainer(LoadedDelay);
			TestEqual(TEXT("Nested FText round-trips"), Label.ToString(), FString(TEXT("Windup")));
			TestTrue(TEXT("Addon FText remains outside module localization"), FTextInspector::GetKey(Label).Get(FString()).IsEmpty());
		}
	}

	const UMASkillModuleGameplayEffectAddon* LoadedEffect =
		Cast<UMASkillModuleGameplayEffectAddon>(LoadedModule.Addons[2]);
	TestNotNull(TEXT("EffectAddon class round-trips"), LoadedEffect);
	if (LoadedEffect)
	{
		const FMASkillModuleGameplayEffectConfig* LoadedConfig =
			GetFirstStruct<FMASkillModuleGameplayEffectConfig>(*LoadedEffect, TEXT("GameplayEffects"));
		TestNotNull(TEXT("Effect config round-trips"), LoadedConfig);
		if (LoadedConfig)
		{
			TestEqual(TEXT("Effect magnitude round-trips"), LoadedConfig->BaseMagnitude, 13.f);
			TestNull(TEXT("Generated EffectDefinition is not source data"), LoadedConfig->GetEffectDefinition());
		}
	}

	const FMASkillDamageConfig* LoadedDamage = LoadedModule.Payloads.Num() > 0
		? LoadedModule.Payloads[0].StructValue.GetPtr<FMASkillDamageConfig>()
		: nullptr;
	TestNotNull(TEXT("InstancedStruct type round-trips"), LoadedDamage);
	if (LoadedDamage) TestEqual(TEXT("InstancedStruct data round-trips"), LoadedDamage->BaseDamage, 42.f);

	FString JsonB;
	if (!TestTrue(
		TEXT("Reserialize loaded data"),
		FMASkillModuleJsonWriter::Write(LoadedModuleId, LoadedModule, JsonB, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Canonical JSON is stable"), JsonB, JsonA);

	int32 SecondModuleId = 0;
	FMASkillModuleData SecondModule;
	TestTrue(
		TEXT("Multiple modules can share one addon owner"),
		FMASkillModuleJsonReader::Read(JsonA, *LoadedOwner, SecondModuleId, SecondModule, Error));
	if (SecondModule.Addons.Num() > 0)
	{
		TestNotEqual(
			TEXT("Shared owner gives each addon a unique name"),
			SecondModule.Addons[0]->GetFName(),
			LoadedModule.Addons[0]->GetFName());
	}
	return !HasAnyErrors();
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMASkillModuleJsonValidationTest,
	"P_MA.Skill.ModuleJson.Validation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMASkillModuleJsonValidationTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UDataTable> AddonOwner(NewObject<UDataTable>());
	FText Error;
	int32 ModuleId = 0;
	FMASkillModuleData ModuleData;

	TestFalse(
		TEXT("ModuleId 0 is rejected"),
		FMASkillModuleJsonReader::Read(
			TEXT("{\"ModuleId\":0,\"Module\":{}}"),
			*AddonOwner,
			ModuleId,
			ModuleData,
			Error));
	TestEqual(TEXT("Failed import returns no module id"), ModuleId, 0);
	TestEqual(TEXT("Failed import returns empty module data"), ModuleData.Addons.Num(), 0);

	const FString WrongClassJson = TEXT(
		"{\"ModuleId\":1,\"Module\":{\"Addons\":[{\"_ClassName\":"
		"\"/Script/P_MA.MASkillSequenceModifier_Delay\"}]}}");
	TestFalse(
		TEXT("Wrong instanced class is rejected"),
		FMASkillModuleJsonReader::Read(WrongClassJson, *AddonOwner, ModuleId, ModuleData, Error));

	const FString DuplicateAddonJson = TEXT(
		"{\"ModuleId\":1,\"Module\":{\"Addons\":["
		"{\"_ClassName\":\"/Script/P_MA.MASkillModuleStackAddon\"},"
		"{\"_ClassName\":\"/Script/P_MA.MASkillModuleStackAddon\"}]}}");
	TestFalse(
		TEXT("Duplicate addon type is rejected"),
		FMASkillModuleJsonReader::Read(DuplicateAddonJson, *AddonOwner, ModuleId, ModuleData, Error));

	TestFalse(
		TEXT("Unknown field is rejected"),
		FMASkillModuleJsonReader::Read(
			TEXT("{\"ModuleId\":1,\"Module\":{\"UnknownField\":1}}"),
			*AddonOwner,
			ModuleId,
			ModuleData,
			Error));
	return !HasAnyErrors();
}

#endif
