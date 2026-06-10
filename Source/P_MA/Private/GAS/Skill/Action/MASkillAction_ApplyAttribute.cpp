#include "GAS/Skill/Action/MASkillAction_ApplyAttribute.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadStore.h"
#include "GameFramework/Actor.h"

namespace
{
	FGameplayAttribute ResolveAttribute(EMADamageAttribute Attribute)
	{
		switch (Attribute)
		{
		case EMADamageAttribute::Health: return UMAAttributeSet::GetHealthAttribute();
		case EMADamageAttribute::MaxHealth: return UMAAttributeSet::GetMaxHealthAttribute();
		case EMADamageAttribute::Attack: return UMAAttributeSet::GetAttackAttribute();
		case EMADamageAttribute::MoveSpeed: return UMAAttributeSet::GetMoveSpeedAttribute();
		case EMADamageAttribute::AttackSpeed: return UMAAttributeSet::GetAttackSpeedAttribute();
		case EMADamageAttribute::Armor: return UMAAttributeSet::GetArmorAttribute();
		case EMADamageAttribute::ArmorPenetration: return UMAAttributeSet::GetArmorPenetrationAttribute();
		case EMADamageAttribute::Focus: return UMAAttributeSet::GetFocusAttribute();
		case EMADamageAttribute::CriticalDamage: return UMAAttributeSet::GetCriticalDamageAttribute();
		case EMADamageAttribute::ReverseCriticalDamage: return UMAAttributeSet::GetReverseCriticalDamageAttribute();
		default: return UMAAttributeSet::GetAttackAttribute();
		}
	}

	float ResolveAttributeCoefficientValue(
		const FMADamageAttributeCoefficient& Coefficient,
		UAbilitySystemComponent& SourceASC,
		UAbilitySystemComponent& TargetASC,
		const FMASkillPayloadStore* PayloadStore)
	{
		if (Coefficient.Side == EMADamageAttributeSide::Payload)
		{
			float PayloadValue = 0.f;
			return PayloadStore && PayloadStore->TryGetScalar(Coefficient.PayloadTag, PayloadValue)
				? PayloadValue * Coefficient.Coefficient
				: 0.f;
		}

		bool bFound = false;
		UAbilitySystemComponent& ASC = Coefficient.Side == EMADamageAttributeSide::Source ? SourceASC : TargetASC;
		const float AttributeValue = ASC.GetGameplayAttributeValue(ResolveAttribute(Coefficient.Attribute), bFound);
		return bFound ? AttributeValue * Coefficient.Coefficient : 0.f;
	}
}

void UMASkillAction_ApplyAttribute::Execute(
	UMASkillAbility& OwnerAbility,
	const FGameplayEventData&,
	const FMASkillEventScopes& Scopes)
{
	if (!OwnerAbility.K2_HasAuthority() || !Attribute.IsValid()) return;

	UAbilitySystemComponent* SourceASC = OwnerAbility.GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	UAbilitySystemComponent* TargetASC = SourceASC;
	if (Target == EMASkillAttributeApplyTarget::TargetPayload)
	{
		if (!Scopes.EventScope || !TargetPayloadTag.IsValid()) return;

		UObject* TargetObject = nullptr;
		if (!Scopes.EventScope->GetPayloadStore().TryGetObject(TargetPayloadTag, TargetObject)) return;

		TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Cast<AActor>(TargetObject));
		if (!TargetASC) return;
	}

	const FMASkillPayloadStore* PayloadStore = Scopes.EventScope ? &Scopes.EventScope->GetPayloadStore() : nullptr;
	float FinalValue = BaseValue;
	for (const FMADamageAttributeCoefficient& Coefficient : AttributeCoefficients)
	{
		FinalValue += ResolveAttributeCoefficientValue(Coefficient, *SourceASC, *TargetASC, PayloadStore);
	}
	if (FMath::IsNearlyZero(FinalValue)) return;

	TargetASC->ApplyModToAttribute(Attribute, Operation, FinalValue);
}
