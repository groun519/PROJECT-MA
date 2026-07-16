#include "GAS/Skill/Addon/Effect/MASkillModuleGameplayEffectAddon.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
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
		ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCallerMagnitude);
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

float FMASkillModuleGameplayEffectConfig::ResolveMagnitude(
	const UAbilitySystemComponent& AbilitySystemComponent,
	const FMASkillPayloadStore& PayloadStore) const
{
	float Magnitude = BaseMagnitude;
	for (const FMAAttributeCoefficient& Coefficient : MagnitudeCoefficients)
	{
		if (FMath::IsNearlyZero(Coefficient.Coefficient)) continue;

		float Value = 0.f;
		if (Coefficient.Source == EMACoefficientSource::Payload)
		{
			if (!PayloadStore.TryGetScalar(Coefficient.PayloadTag, Value)) continue;
		}
		else
		{
			bool bFound = false;
			Value = AbilitySystemComponent.GetGameplayAttributeValue(Coefficient.GameplayAttribute, bFound);
			if (!bFound) continue;
		}

		Magnitude += Value * Coefficient.Coefficient;
	}
	return Magnitude;
}

void UMASkillModuleGameplayEffectAddon::BindModule(UMASkillModuleInstance& ModuleInstance) const
{
	const TWeakObjectPtr<UMASkillModuleInstance> WeakModuleInstance = &ModuleInstance;
	ModuleInstance.OnStateChanged.AddWeakLambda(&ModuleInstance, [this, WeakModuleInstance]
	{
		if (UMASkillModuleInstance* BoundModuleInstance = WeakModuleInstance.Get())
		{
			SyncEffect(*BoundModuleInstance);
		}
	});
}

void UMASkillModuleGameplayEffectAddon::SyncEffect(UMASkillModuleInstance& ModuleInstance) const
{
	AActor* OwnerActor = ModuleInstance.GetTypedOuter<AActor>();
	if (!OwnerActor || !OwnerActor->HasAuthority()) return;

	UAbilitySystemComponent* AbilitySystemComponent =
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	if (!AbilitySystemComponent) return;
	const FMASkillPayloadStore& PayloadStore = ModuleInstance.GetPayloadStore();

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

		if (!ModuleInstance.IsAssemblyActive())
		{
			AbilitySystemComponent->RemoveActiveEffects(Query);
			continue;
		}

		const FGameplayTag MagnitudeTag = UMAAbilitySystemStatics::GetModuleEffectMagnitudeTag();
		const float Magnitude = Config.ResolveMagnitude(*AbilitySystemComponent, PayloadStore);

		const TArray<FActiveGameplayEffectHandle> EffectHandles = AbilitySystemComponent->GetActiveEffects(Query);
		if (!EffectHandles.IsEmpty())
		{
			AbilitySystemComponent->UpdateActiveGameplayEffectSetByCallerMagnitude(
				EffectHandles[0],
				MagnitudeTag,
				Magnitude);
			continue;
		}

		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(&ModuleInstance);

		FGameplayEffectSpec EffectSpec(GameplayEffect, EffectContext, 1.f);
		EffectSpec.DynamicGrantedTags.AppendTags(Config.GrantedTags);
		EffectSpec.SetSetByCallerMagnitude(MagnitudeTag, Magnitude);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(EffectSpec);
	}
}
