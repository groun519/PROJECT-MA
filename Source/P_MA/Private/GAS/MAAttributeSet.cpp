#include "GAS/MAAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectAggregatorLibrary.h"
#include "GAS/MAAbilitySystemComponent.h"

/*
* void UNVAttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
* {
* 	GAMEPLAYATTRIBUTE_REPNOTIFY(UNVAttributeSet, Health, OldValue)
* }
*
* ^ ^ ^ ^ ^ ^ ^ ^ ^ ^
*
* DEFINE_REPNOTIFY(Health)
*/
#define DEFINE_REPNOTIFY(PropertyName)                              \
void UMAAttributeSet::OnRep_##PropertyName(                     \
const FGameplayAttributeData& OldValue)                     \
{                                                               \
GAMEPLAYATTRIBUTE_REPNOTIFY(UMAAttributeSet, PropertyName, OldValue); \
}

UMAAttributeSet::UMAAttributeSet()
	: SlowMultiplier(1.f)
	, CriticalDamage(1.5f)
	, ReverseCriticalDamage(0.5f)
{}

void UMAAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Shield, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Attack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, SlowMultiplier, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Focus, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, CriticalDamage, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, ReverseCriticalDamage, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Temperature, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Coin, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, AttackRange, COND_None, REPNOTIFY_Always)
}

void UMAAttributeSet::OnAttributeAggregatorCreated(const FGameplayAttribute& Attribute, FAggregator* NewAggregator) const
{
	Super::OnAttributeAggregatorCreated(Attribute, NewAggregator);

	if (Attribute == GetSlowMultiplierAttribute())
	{
		NewAggregator->EvaluationMetaData = &FAggregatorEvaluateMetaDataLibrary::MostNegativeMod_AllPositiveMods;
	}
}

void UMAAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	else if (Attribute == GetShieldAttribute())
		NewValue = FMath::Max(NewValue, 0.f);
	else if (Attribute == GetSlowMultiplierAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	else if (Attribute == GetFocusAttribute())
		NewValue = FMath::Clamp(NewValue, -1.f, 1.f);
	else if (Attribute == GetTemperatureAttribute())
		NewValue = FMath::Clamp(NewValue, -100.f, 100.f);
}

void UMAAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		SetShield(FMath::Max(GetShield(), 0.f));
		return;
	}
	else if (Data.EvaluatedData.Attribute == GetTemperatureAttribute())
	{
		SetTemperature(FMath::Clamp(GetTemperature(), -100.f, 100.f));
	}
	else return;

	if (UMAAbilitySystemComponent* TargetASC = Cast<UMAAbilitySystemComponent>(&Data.Target))
	{
		TargetASC->NotifyDamageAppliedFromGameplayEffect(Data);
	}
}

DEFINE_REPNOTIFY(Health)
DEFINE_REPNOTIFY(MaxHealth)
DEFINE_REPNOTIFY(Shield)
DEFINE_REPNOTIFY(Attack)
DEFINE_REPNOTIFY(MoveSpeed)
DEFINE_REPNOTIFY(SlowMultiplier)
DEFINE_REPNOTIFY(AttackSpeed)
DEFINE_REPNOTIFY(Armor)
DEFINE_REPNOTIFY(ArmorPenetration)
DEFINE_REPNOTIFY(Focus)
DEFINE_REPNOTIFY(CriticalDamage)
DEFINE_REPNOTIFY(ReverseCriticalDamage)
DEFINE_REPNOTIFY(Temperature)
DEFINE_REPNOTIFY(Coin)
DEFINE_REPNOTIFY(AttackRange)
