#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GAS/Skill/Action/MASkillAction_MeleeOverlap.h"
#include "GAS/Skill/Addon/Effect/MASkillModuleGameplayEffectAddon.h"
#include "GAS/Skill/Addon/Event/MASkillModuleEventBindingAddon.h"
#include "GAS/Skill/Addon/Sequence/MASkillModuleSequenceAddon.h"
#include "GAS/Skill/Addon/Stack/MASkillModuleStackAddon.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Module/MASkillModuleDataTypes.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonReader.h"
#include "GAS/Skill/Module/Json/MASkillModuleJsonWriter.h"
#include "GAS/Skill/Sequence/Variants/MASkillSequenceModifier_Delay.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect_Attribute.h"
#include "GAS/Skill/StatusEffect/MASkillStatusEffect_Impulse.h"
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

	template<typename StructType>
	void SetStructProperty(UObject& Object, const FName PropertyName, const StructType& Value)
	{
		FStructProperty* Property = FindPropertyChecked<FStructProperty>(*Object.GetClass(), PropertyName);
		check(Property->Struct == StructType::StaticStruct());
		*Property->ContainerPtrToValuePtr<StructType>(&Object) = Value;
	}

	template<typename StructType>
	const StructType& GetStructProperty(const UObject& Object, const FName PropertyName)
	{
		FStructProperty* Property = FindPropertyChecked<FStructProperty>(*Object.GetClass(), PropertyName);
		check(Property->Struct == StructType::StaticStruct());
		return *Property->ContainerPtrToValuePtr<StructType>(&Object);
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
	const FGameplayTag AttackTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Attack"));
	const FGameplayTag MovementTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Movement"));
	const FGameplayTag VisualTag = FGameplayTag::RequestGameplayTag(TEXT("Module.Visual.Elemental.Fire"));
	const FGameplayTag HitEventTag = FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.Hit"));
	const FGameplayTag DamagePayloadTag = FGameplayTag::RequestGameplayTag(TEXT("Data.Damage.Base"));
	const FGameplayTag FireDamageTag = FGameplayTag::RequestGameplayTag(TEXT("DamageType.Fire"));
	const FGameplayTag HitCueTag = FGameplayTag::RequestGameplayTag(TEXT("GameplayCue.Hit.Overlay.Red"));
	const FGameplayTag StunTag = FGameplayTag::RequestGameplayTag(TEXT("State.Debuff.Stun"));
	Module.ModuleTags.AddTag(MovementTag);
	Module.ModuleTags.AddTag(AttackTag);
	Module.ModuleVisualTags.AddTag(VisualTag);

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
	EffectConfig.GrantedTags.AddTag(StunTag);
	EffectConfig.GameplayCueTags.AddTag(HitCueTag);
	FPropertyChangedEvent EffectPropertyChanged(
		FindPropertyChecked<FArrayProperty>(*EffectAddon->GetClass(), TEXT("GameplayEffects")));
	EffectAddon->PostEditChangeProperty(EffectPropertyChanged);
	TestNotNull(TEXT("Generated EffectDefinition exists before export"), EffectConfig.GetEffectDefinition());
	Module.Addons.Add(EffectAddon);

	UMASkillModuleEventBindingAddon* EventBindingAddon =
		NewObject<UMASkillModuleEventBindingAddon>(SourceOwner.Get());
	FMASkillEventBinding& EventBinding =
		AddStructToArray<FMASkillEventBinding>(*EventBindingAddon, TEXT("EventBindings"));
	EventBinding.EventTag = HitEventTag;
	UMASkillAction_MeleeOverlap* MeleeAction = NewObject<UMASkillAction_MeleeOverlap>(EventBindingAddon);
	SetStructProperty(*MeleeAction, TEXT("DamagePayloadTag"), DamagePayloadTag);
	EventBinding.Action = MeleeAction;
	AddStructToArray<FMASkillEventBinding>(*EventBindingAddon, TEXT("EventBindings"));
	Module.Addons.Add(EventBindingAddon);

	FMASkillPayloadEntry& Payload = Module.Payloads.AddDefaulted_GetRef();
	Payload.PayloadTag = DamagePayloadTag;
	Payload.ValueType = EMASkillPayloadValueType::Struct;
	Payload.StructValue.InitializeAs<FMASkillDamageConfig>();
	FMASkillDamageConfig& DamageConfig = Payload.StructValue.GetMutable<FMASkillDamageConfig>();
	DamageConfig.BaseDamage = 42.f;
	DamageConfig.DamageTypeTag = FireDamageTag;
	DamageConfig.TargetGameplayCueTags.AddTag(HitCueTag);
	UMASkillStatusEffectKnockback* StatusEffect =
		NewObject<UMASkillStatusEffectKnockback>(SourceOwner.Get());
	SetProperty<float, FFloatProperty>(*StatusEffect, TEXT("Magnitude"), 321.f);
	DamageConfig.StatusEffects.Add(StatusEffect);
	UMASkillStatusEffect_Slow* SlowStatusEffect = NewObject<UMASkillStatusEffect_Slow>(SourceOwner.Get());
	FGameplayTagContainer GrantedTags;
	GrantedTags.AddTag(StunTag);
	SetStructProperty(*SlowStatusEffect, TEXT("GrantedTags"), GrantedTags);
	DamageConfig.StatusEffects.Add(SlowStatusEffect);
	const FString SourceStatusEffectPath = StatusEffect->GetPathName();

	FString JsonA;
	FText Error;
	if (!TestTrue(TEXT("Serialize module data"), FMASkillModuleJsonWriter::Write(1201, Module, JsonA, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestTrue(TEXT("JSON contains polymorphic class data"), JsonA.Contains(TEXT("_ClassName")));
	TestTrue(TEXT("JSON contains instanced struct type data"), JsonA.Contains(TEXT("_StructName")));
	TestFalse(TEXT("JSON does not reference the source payload object"), JsonA.Contains(SourceStatusEffectPath));
	TestTrue(TEXT("JSON contains plain text"), JsonA.Contains(TEXT("\"DisplayName\": \"Slash\"")));
	TestTrue(TEXT("JSON contains gameplay tag strings"), JsonA.Contains(TEXT("\"DamageTypeTag\": \"DamageType.Fire\"")));
	TestTrue(TEXT("JSON contains gameplay tag arrays"), JsonA.Contains(TEXT("\"ModuleTags\": [")));
	TestFalse(TEXT("JSON excludes text identity fields"), JsonA.Contains(TEXT("\"Key\"")));
	TestFalse(TEXT("Generated EffectDefinition is excluded"), JsonA.Contains(TEXT("EffectDefinition")));
	TestTrue(TEXT("Null instanced objects serialize as null"), JsonA.Contains(TEXT("\"action\": null")));

	FMASkillModuleJsonHeader Header;
	TestTrue(
		TEXT("Read module header without importing module data"),
		FMASkillModuleJsonReader::ReadHeader(
			JsonA,
			Header,
			Error));
	TestEqual(TEXT("Header contains ModuleId"), Header.ModuleId, 1201);
	TestEqual(TEXT("Header contains ModuleName"), Header.ModuleName, FName(TEXT("JsonRoundTrip")));
	TestEqual(TEXT("Header contains module rarity"), Header.ModuleRarity, EMAModuleRarity::Rarity4);
	TestEqual(TEXT("Header contains module type"), Header.ModuleType, EMASkillModuleType::Module);

	TStrongObjectPtr<UDataTable> LoadedOwner(NewObject<UDataTable>());
	FMASkillModuleReadResult ReadResult = FMASkillModuleJsonReader::Read(JsonA, *LoadedOwner);
	if (!TestTrue(
		TEXT("Deserialize module data"),
		ReadResult.IsValid()))
	{
		AddError(ReadResult.GetDiagnosticsText().ToString());
		return false;
	}
	const int32 LoadedModuleId = ReadResult.ModuleId;
	FMASkillModuleData LoadedModule = MoveTemp(ReadResult.ModuleData);
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
	TestTrue(TEXT("Module tag container round-trips"), LoadedModule.ModuleTags.HasTagExact(AttackTag));
	TestTrue(TEXT("Module tag order does not lose values"), LoadedModule.ModuleTags.HasTagExact(MovementTag));
	TestTrue(TEXT("Visual tag container round-trips"), LoadedModule.ModuleVisualTags.HasTagExact(VisualTag));
	TestEqual(TEXT("Addon count round-trips"), LoadedModule.Addons.Num(), 4);
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
			TestTrue(TEXT("Effect granted tags round-trip"), LoadedConfig->GrantedTags.HasTagExact(StunTag));
			TestTrue(TEXT("Effect gameplay cue tags round-trip"), LoadedConfig->GameplayCueTags.HasTagExact(HitCueTag));
			TestNull(TEXT("Generated EffectDefinition is not source data"), LoadedConfig->GetEffectDefinition());
		}
	}

	const UMASkillModuleEventBindingAddon* LoadedEventBinding =
		Cast<UMASkillModuleEventBindingAddon>(LoadedModule.Addons[3]);
	TestNotNull(TEXT("EventBindingAddon class round-trips"), LoadedEventBinding);
	if (LoadedEventBinding)
	{
		TestEqual(TEXT("Event binding count round-trips"), LoadedEventBinding->GetEventBindings().Num(), 2);
		if (LoadedEventBinding->GetEventBindings().Num() == 2)
		{
			const FMASkillEventBinding& LoadedBinding = LoadedEventBinding->GetEventBindings()[0];
			TestEqual(TEXT("Event tag round-trips"), LoadedBinding.EventTag, HitEventTag);
			const UMASkillAction_MeleeOverlap* LoadedAction = Cast<UMASkillAction_MeleeOverlap>(LoadedBinding.Action);
			TestNotNull(TEXT("Nested action class round-trips"), LoadedAction);
			if (LoadedAction)
			{
				TestEqual(
					TEXT("Nested action payload tag round-trips"),
					GetStructProperty<FGameplayTag>(*LoadedAction, TEXT("DamagePayloadTag")),
					DamagePayloadTag);
			}
			TestNull(TEXT("Null action round-trips"), LoadedEventBinding->GetEventBindings()[1].Action);
		}
	}

	const FMASkillDamageConfig* LoadedDamage = LoadedModule.Payloads.Num() > 0
		? LoadedModule.Payloads[0].StructValue.GetPtr<FMASkillDamageConfig>()
		: nullptr;
	TestNotNull(TEXT("InstancedStruct type round-trips"), LoadedDamage);
	if (LoadedDamage)
	{
		TestEqual(TEXT("InstancedStruct data round-trips"), LoadedDamage->BaseDamage, 42.f);
		TestEqual(TEXT("Damage type tag round-trips"), LoadedDamage->DamageTypeTag, FireDamageTag);
		TestTrue(TEXT("Damage gameplay cue tags round-trip"), LoadedDamage->TargetGameplayCueTags.HasTagExact(HitCueTag));
		TestEqual(TEXT("Nested instanced object count round-trips"), LoadedDamage->StatusEffects.Num(), 2);
		const UMASkillStatusEffectKnockback* LoadedStatusEffect =
			LoadedDamage->StatusEffects.Num() > 0
				? Cast<UMASkillStatusEffectKnockback>(LoadedDamage->StatusEffects[0])
				: nullptr;
		TestNotNull(TEXT("Nested instanced object class round-trips"), LoadedStatusEffect);
		if (LoadedStatusEffect)
		{
			TestTrue(TEXT("Nested instanced object uses the requested owner"), LoadedStatusEffect->IsIn(LoadedOwner.Get()));
			const FFloatProperty* MagnitudeProperty =
				FindPropertyChecked<FFloatProperty>(*LoadedStatusEffect->GetClass(), TEXT("Magnitude"));
			TestEqual(
				TEXT("Nested instanced object data round-trips"),
				MagnitudeProperty->GetPropertyValue_InContainer(LoadedStatusEffect),
				321.f);
		}
		const UMASkillStatusEffect_Slow* LoadedSlow = LoadedDamage->StatusEffects.Num() > 1
			? Cast<UMASkillStatusEffect_Slow>(LoadedDamage->StatusEffects[1])
			: nullptr;
		TestNotNull(TEXT("Nested attribute status effect round-trips"), LoadedSlow);
		if (LoadedSlow)
		{
			TestTrue(
				TEXT("Nested status effect tags round-trip"),
				GetStructProperty<FGameplayTagContainer>(*LoadedSlow, TEXT("GrantedTags")).HasTagExact(StunTag));
		}
	}

	FString JsonB;
	if (!TestTrue(
		TEXT("Reserialize loaded data"),
		FMASkillModuleJsonWriter::Write(LoadedModuleId, LoadedModule, JsonB, Error)))
	{
		AddError(Error.ToString());
		return false;
	}
	TestEqual(TEXT("Canonical JSON is stable"), JsonB, JsonA);

	FMASkillModuleReadResult SecondReadResult = FMASkillModuleJsonReader::Read(JsonA, *LoadedOwner);
	TestTrue(
		TEXT("Multiple modules can share one addon owner"),
		SecondReadResult.IsValid());
	FMASkillModuleData SecondModule = MoveTemp(SecondReadResult.ModuleData);
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

	FMASkillModuleReadResult ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":0,\"Module\":{}}"),
		*AddonOwner);
	TestFalse(TEXT("ModuleId 0 is rejected"), ReadResult.IsValid());
	TestEqual(TEXT("Failed import returns no module id"), ReadResult.ModuleId, 0);
	TestEqual(TEXT("Failed import returns empty module data"), ReadResult.ModuleData.Addons.Num(), 0);
	if (TestFalse(TEXT("Invalid ModuleId returns a diagnostic"), ReadResult.Diagnostics.IsEmpty()))
	{
		TestEqual(
			TEXT("ModuleId diagnostic identifies its field"),
			ReadResult.Diagnostics[0].Path,
			FString(TEXT("ModuleId")));
	}

	const FString WrongClassJson = TEXT(
		"{\"ModuleId\":1,\"Module\":{\"Addons\":[{\"_ClassName\":"
		"\"/Script/P_MA.MASkillSequenceModifier_Delay\"}]}}");
	ReadResult = FMASkillModuleJsonReader::Read(WrongClassJson, *AddonOwner);
	TestFalse(TEXT("Wrong instanced class is rejected"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"DisplayData\":{\"IconData\":{\"Icon\":1}}}}"),
		*AddonOwner);
	TestFalse(TEXT("Object reference with a non-string value is rejected"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"Addons\":[\"/Script/P_MA.Default__MASkillModuleStackAddon\"]}}"),
		*AddonOwner);
	TestFalse(TEXT("Instanced addon reference is rejected"), ReadResult.IsValid());

	const FString DuplicateAddonJson = TEXT(
		"{\"ModuleId\":1,\"Module\":{\"Addons\":["
		"{\"_ClassName\":\"/Script/P_MA.MASkillModuleStackAddon\"},"
		"{\"_ClassName\":\"/Script/P_MA.MASkillModuleStackAddon\"}]}}");
	ReadResult = FMASkillModuleJsonReader::Read(DuplicateAddonJson, *AddonOwner);
	TestFalse(TEXT("Duplicate addon type is rejected"), ReadResult.IsValid());
	if (TestFalse(TEXT("Duplicate addon returns a diagnostic"), ReadResult.Diagnostics.IsEmpty()))
	{
		TestEqual(
			TEXT("Duplicate addon diagnostic identifies its field"),
			ReadResult.Diagnostics[0].Path,
			FString(TEXT("Module.Addons[1]")));
	}

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleType\":\"Sub\",\"Addons\":[")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleStackAddon\"}]}}"),
		*AddonOwner);
	TestFalse(TEXT("Sub module rejects a root-only addon"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleType\":\"Sub\",\"Addons\":[")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleGameplayEffectAddon\"}]}}"),
		*AddonOwner);
	TestTrue(TEXT("Sub module accepts a supported addon"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleType\":\"Item\"}}"),
		*AddonOwner);
	TestFalse(TEXT("Item module requires an Item addon"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleType\":\"Item\",\"Addons\":[")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleItemAddon\"}]}}"),
		*AddonOwner);
	TestFalse(TEXT("Item module requires a use binding"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleType\":\"Item\",\"Addons\":[")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleItemAddon\"},")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleEventBindingAddon\",\"EventBindings\":[{")
		TEXT("\"EventTag\":\"Event.Item.Use\",\"BindingScope\":\"Global\"}]}]}}"),
		*AddonOwner);
	TestFalse(TEXT("Item use binding requires an action"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleType\":\"Item\",\"Addons\":[")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleItemAddon\"},")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleEventBindingAddon\",\"EventBindings\":[{")
		TEXT("\"EventTag\":\"Event.Item.Use\",\"BindingScope\":\"Global\",\"Action\":{")
		TEXT("\"_ClassName\":\"/Script/P_MA.MASkillAction_MeleeOverlap\"}}]}]}}"),
		*AddonOwner);
	TestFalse(TEXT("Item module rejects an unsupported action"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleType\":\"Item\",\"Addons\":[")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleItemAddon\"},")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleEventBindingAddon\",\"EventBindings\":[{")
		TEXT("\"EventTag\":\"Event.Item.Use\",\"BindingScope\":\"Module\",\"Action\":{")
		TEXT("\"_ClassName\":\"/Script/P_MA.MASkillAction_ApplyGameplayEffectToSelf\"}}]}]}}"),
		*AddonOwner);
	TestFalse(TEXT("Item module rejects scoped event bindings"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleType\":\"Item\",\"Addons\":[")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleItemAddon\"},")
		TEXT("{\"_ClassName\":\"/Script/P_MA.MASkillModuleEventBindingAddon\",\"EventBindings\":[{")
		TEXT("\"EventTag\":\"Event.Item.Use\",\"BindingScope\":\"Global\",\"Action\":{")
		TEXT("\"_ClassName\":\"/Script/P_MA.MASkillAction_ApplyGameplayEffectToSelf\"}}]}]}}"),
		*AddonOwner);
	TestTrue(TEXT("Item module accepts a supported action"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"UnknownField\":1}}"),
		*AddonOwner);
	TestFalse(TEXT("Unknown field is rejected"), ReadResult.IsValid());
	if (TestFalse(TEXT("Unknown field returns a diagnostic"), ReadResult.Diagnostics.IsEmpty()))
	{
		TestEqual(
			TEXT("Unknown field diagnostic identifies its field"),
			ReadResult.Diagnostics[0].Path,
			FString(TEXT("Module.UnknownField")));
	}

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleTags\":{}}}"),
		*AddonOwner);
	TestFalse(TEXT("Gameplay tag container object is rejected"), ReadResult.IsValid());
	if (TestFalse(TEXT("Invalid gameplay tag container returns a diagnostic"), ReadResult.Diagnostics.IsEmpty()))
	{
		TestEqual(
			TEXT("Gameplay tag container diagnostic identifies its field"),
			ReadResult.Diagnostics[0].Path,
			FString(TEXT("Module.ModuleTags")));
	}

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleTags\":[\"Module.DoesNotExist\"]}}"),
		*AddonOwner);
	TestFalse(TEXT("Unknown gameplay tag is rejected"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"ModuleTags\":[\"Module.Attack\",\"Module.Attack\"]}}"),
		*AddonOwner);
	TestFalse(TEXT("Duplicate gameplay tag is rejected"), ReadResult.IsValid());

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"Payloads\":[{\"structValue\":{\"_StructName\":" )
		TEXT("\"/Script/P_MA.MASkillDamageConfig\",\"Value\":{\"DamageTypeTag\":{}}}}]}}"),
		*AddonOwner);
	TestFalse(TEXT("Gameplay tag object inside payload is rejected"), ReadResult.IsValid());
	if (TestFalse(TEXT("Invalid payload tag returns a diagnostic"), ReadResult.Diagnostics.IsEmpty()))
	{
		TestEqual(
			TEXT("Nested gameplay tag diagnostic identifies its field"),
			ReadResult.Diagnostics[0].Path,
			FString(TEXT("Module.Payloads[0].structValue.Value.DamageTypeTag")));
	}

	ReadResult = FMASkillModuleJsonReader::Read(
		TEXT("{\"ModuleId\":1,\"Module\":{\"Payloads\":[{\"structValue\":{\"_StructName\":")
		TEXT("\"/Script/CoreUObject.Vector\",\"Value\":{}}}]}}"),
		*AddonOwner);
	TestFalse(TEXT("Non-payload InstancedStruct type is rejected"), ReadResult.IsValid());
	return !HasAnyErrors();
}

#endif
