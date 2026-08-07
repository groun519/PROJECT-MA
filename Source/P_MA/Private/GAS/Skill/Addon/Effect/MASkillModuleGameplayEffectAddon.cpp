#include "GAS/Skill/Addon/Effect/MASkillModuleGameplayEffectAddon.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccess.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GameFramework/Actor.h"

#if WITH_EDITOR
void UMASkillModuleGameplayEffectAddon::BuildGeneratedData()
{
	RebuildEffectDefinitions();
}

void FMASkillModuleGameplayEffectConfig::RebuildEffectDefinition(UObject& Outer)
{
	if (!EffectDefinition)
	{
		EffectDefinition = NewObject<UGameplayEffect>(&Outer, NAME_None, RF_Transactional);
	}

	FSetByCallerFloat SetByCallerMagnitude;
	SetByCallerMagnitude.DataTag = UMAAbilitySystemStatics::GetModuleEffectMagnitudeTag();

	EffectDefinition->DurationPolicy = EGameplayEffectDurationType::Infinite;
	EffectDefinition->Modifiers.Reset(Modifiers.Num());
	for (const FMASkillModuleGameplayModifier& Modifier : Modifiers)
	{
		if (!Modifier.Attribute.IsValid()) continue;

		FGameplayModifierInfo& ModifierInfo = EffectDefinition->Modifiers.AddDefaulted_GetRef();
		ModifierInfo.Attribute = Modifier.Attribute;
		ModifierInfo.ModifierOp = Modifier.ModifierOp;
		if (MagnitudeType == EMASkillModuleEffectMagnitudeType::AttributeBased
			&& MagnitudeAttribute.IsValid())
		{
			FAttributeBasedFloat AttributeMagnitude;
			AttributeMagnitude.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
				MagnitudeAttribute,
				EGameplayEffectAttributeCaptureSource::Source,
				false);
			AttributeMagnitude.Coefficient = FScalableFloat(MagnitudeAttributeCoefficient);
			AttributeMagnitude.PostMultiplyAdditiveValue = FScalableFloat(BaseMagnitude);
			ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(AttributeMagnitude);
		}
		else
		{
			ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);
		}
	}
	EffectDefinition->Period = FScalableFloat(Period);
	EffectDefinition->bExecutePeriodicEffectOnApplication = bExecutePeriodicEffectOnApplication;
	EffectDefinition->GameplayCues.Reset();
	if (!GameplayCueTags.IsEmpty())
	{
		EffectDefinition->GameplayCues.AddDefaulted_GetRef().GameplayCueTags = GameplayCueTags;
	}
}

void UMASkillModuleGameplayEffectAddon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	RebuildEffectDefinitions();
}

void UMASkillModuleGameplayEffectAddon::RebuildEffectDefinitions()
{
	for (FMASkillModuleGameplayEffectConfig& Config : GameplayEffects)
	{
		Config.RebuildEffectDefinition(*this);
	}
}
#endif

UMASkillModuleAddon* UMASkillModuleGameplayEffectAddon::AssembleInto(
	UObject& ResultOuter,
	UMASkillModuleAddon* ResultAddon,
	const EMASkillAddonAssemblyStage Stage,
	const FMASkillScopes&) const
{
	if (Stage != EMASkillAddonAssemblyStage::ModuleComposition || GameplayEffects.IsEmpty())
	{
		return ResultAddon;
	}

	UMASkillModuleGameplayEffectAddon* Result = ResultAddon
		? static_cast<UMASkillModuleGameplayEffectAddon*>(ResultAddon)
		: NewObject<UMASkillModuleGameplayEffectAddon>(&ResultOuter, GetClass());
	// Generated GE definitions remain owned by their built module assets.
	Result->GameplayEffects.Append(GameplayEffects);
	return Result;
}

float FMASkillModuleGameplayEffectConfig::ResolveMagnitude(
	const UAbilitySystemComponent& AbilitySystemComponent,
	const FMASkillPayloadStore& PayloadStore) const
{
	float Magnitude = BaseMagnitude;
	const FMASkillPayloadAccess Payloads(nullptr, nullptr, &PayloadStore);
	for (const FMAAttributeCoefficient& Coefficient : MagnitudeCoefficients)
	{
		if (FMath::IsNearlyZero(Coefficient.Coefficient)) continue;
		Magnitude += Coefficient.ResolveValue(AbilitySystemComponent, AbilitySystemComponent, Payloads);
	}
	return Magnitude;
}

void UMASkillModuleGameplayEffectAddon::BindModule(UMASkillModuleInstance& ModuleInstance) const
{
	ModuleInstance.OnStateChanged.RemoveAll(this);
	const TWeakObjectPtr<UMASkillModuleInstance> WeakModuleInstance = &ModuleInstance;
	ModuleInstance.OnStateChanged.AddWeakLambda(this, [this, WeakModuleInstance]
	{
		if (UMASkillModuleInstance* BoundModuleInstance = WeakModuleInstance.Get())
		{
			SetEffectsActive(*BoundModuleInstance, BoundModuleInstance->IsAssemblyActive());
		}
	});
	SetEffectsActive(ModuleInstance, ModuleInstance.IsAssemblyActive());
}

void UMASkillModuleGameplayEffectAddon::UnbindModule(UMASkillModuleInstance& ModuleInstance) const
{
	ModuleInstance.OnStateChanged.RemoveAll(this);
	SetEffectsActive(ModuleInstance, false);
}

void UMASkillModuleGameplayEffectAddon::SetEffectsActive(
	UMASkillModuleInstance& ModuleInstance,
	const bool bActive) const
{
	AActor* OwnerActor = ModuleInstance.GetTypedOuter<AActor>();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!AbilitySystemComponent) return;
	const FMASkillPayloadStore& PayloadStore = ModuleInstance.GetPayloadStore();
	TSet<FActiveGameplayEffectHandle> ClaimedEffectHandles;

	for (const FMASkillModuleGameplayEffectConfig& Config : GameplayEffects)
	{
		const UGameplayEffect* GameplayEffect = Config.GetEffectDefinition();
		if (!GameplayEffect) continue;

		FGameplayEffectQuery Query;
		Query.EffectSource = &ModuleInstance;
		Query.CustomMatchDelegate.BindLambda([GameplayEffect](const FActiveGameplayEffect& ActiveEffect)
		{
			return ActiveEffect.Spec.Def == GameplayEffect;
		});

		if (!bActive)
		{
			AbilitySystemComponent->RemoveActiveEffects(Query);
			continue;
		}

		const TArray<FActiveGameplayEffectHandle> EffectHandles = AbilitySystemComponent->GetActiveEffects(Query);
		const FActiveGameplayEffectHandle* ExistingHandle = EffectHandles.FindByPredicate(
			[&ClaimedEffectHandles](const FActiveGameplayEffectHandle& Handle)
			{
				return !ClaimedEffectHandles.Contains(Handle);
			});
		if (ExistingHandle)
		{
			if (Config.MagnitudeType == EMASkillModuleEffectMagnitudeType::Snapshot)
			{
				AbilitySystemComponent->UpdateActiveGameplayEffectSetByCallerMagnitude(
					*ExistingHandle,
					UMAAbilitySystemStatics::GetModuleEffectMagnitudeTag(),
					Config.ResolveMagnitude(*AbilitySystemComponent, PayloadStore));
			}
			ClaimedEffectHandles.Add(*ExistingHandle);
			continue;
		}

		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(&ModuleInstance);

		FGameplayEffectSpec EffectSpec(GameplayEffect, EffectContext, 1.f);
		EffectSpec.DynamicGrantedTags.AppendTags(Config.GrantedTags);
		if (Config.MagnitudeType == EMASkillModuleEffectMagnitudeType::Snapshot)
		{
			EffectSpec.SetSetByCallerMagnitude(
				UMAAbilitySystemStatics::GetModuleEffectMagnitudeTag(),
				Config.ResolveMagnitude(*AbilitySystemComponent, PayloadStore));
		}
		const FActiveGameplayEffectHandle AppliedHandle =
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(EffectSpec);
		if (AppliedHandle.IsValid()) ClaimedEffectHandles.Add(AppliedHandle);
	}
}
