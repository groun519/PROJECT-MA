#include "GAS/Skill/Action/MASkillAction_ApplyAttribute.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "GAS/Skill/Payload/MASkillPayloadAccessor.h"
#include "GameFramework/Actor.h"

namespace
{
	const FName LostHealthPayloadTagName(TEXT("Data.Health.Lost"));

	float ResolveAttributeCoefficientValue(
		const FMAAttributeCoefficient& Coefficient,
		UAbilitySystemComponent& SourceASC,
		UAbilitySystemComponent& TargetASC,
		const FMASkillPayloadAccessor& Payloads)
	{
		if (Coefficient.Source == EMACoefficientSource::Payload)
		{
			float PayloadValue = 0.f;
			return Payloads.TryGetScalar(Coefficient.PayloadTag, PayloadValue)
				? PayloadValue * Coefficient.Coefficient
				: 0.f;
		}
		if (!Coefficient.GameplayAttribute.IsValid()) return 0.f;

		bool bFound = false;
		UAbilitySystemComponent& ASC = Coefficient.Source == EMACoefficientSource::Source ? SourceASC : TargetASC;
		const float AttributeValue = ASC.GetGameplayAttributeValue(Coefficient.GameplayAttribute, bFound);
		return bFound ? AttributeValue * Coefficient.Coefficient : 0.f;
	}
}

void UMASkillAction_ApplyAttribute::PostLoad()
{
	Super::PostLoad();

	for (int32 Index = AttributeCoefficients.Num() - 1; Index >= 0; --Index)
	{
		const FMAAttributeCoefficient& Coefficient = AttributeCoefficients[Index];
		if (Coefficient.Source != EMACoefficientSource::Payload
			|| Coefficient.PayloadTag.GetTagName() != LostHealthPayloadTagName)
		{
			continue;
		}

		EventMagnitudeCoefficient += Coefficient.Coefficient;
		AttributeCoefficients.RemoveAt(Index);
	}
}

void UMASkillAction_ApplyAttribute::Execute(
	UMASkillAbility& OwnerAbility,
	const FMASkillEvent& Event,
	const FMASkillScopes& Scopes)
{
	if (!OwnerAbility.K2_HasAuthority() || !Attribute.IsValid()) return;

	UAbilitySystemComponent* SourceASC = OwnerAbility.GetAbilitySystemComponentFromActorInfo();
	if (!SourceASC) return;

	const FMASkillPayloadAccessor Payloads = Event.GetPayloadAccess(Scopes);
	UAbilitySystemComponent* TargetASC = SourceASC;
	if (Target == EMASkillAttributeApplyTarget::TargetPayload)
	{
		if (!TargetPayloadTag.IsValid()) return;

		UObject* TargetObject = nullptr;
		if (!Payloads.TryGetObject(TargetPayloadTag, TargetObject)) return;

		TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Cast<AActor>(TargetObject));
		if (!TargetASC) return;
	}

	float FinalValue = BaseValue + Event.GetMagnitude() * EventMagnitudeCoefficient;
	for (const FMAAttributeCoefficient& Coefficient : AttributeCoefficients)
	{
		FinalValue += ResolveAttributeCoefficientValue(Coefficient, *SourceASC, *TargetASC, Payloads);
	}
	if (FMath::IsNearlyZero(FinalValue)) return;

	TargetASC->ApplyModToAttribute(Attribute, Operation, FinalValue);
}
