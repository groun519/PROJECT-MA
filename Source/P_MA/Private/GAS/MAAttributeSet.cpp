#include "GAS/MAAttributeSet.h"

#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "MAGameplayAbilityTypes.h"
#include "GAS/Skill/MASkillAbility.h"
#include "GAS/Skill/Module/MASkillModuleInstance.h"
#include "Player/MAPlayerController.h"

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

void UMAAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Attack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, DamageVariance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MoveSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, AttackSpeed, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, Fury, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, MaxFury, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, CriticalChance, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(UMAAttributeSet, CriticalDamage, COND_None, REPNOTIFY_Always)
}

void UMAAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
}

void UMAAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));

	float DeltaHealth = Data.EvaluatedData.Magnitude;

	if (DeltaHealth < 0.f)
	{
		float FinalDamage = -DeltaHealth;

		bool bIsCriticalHit = false;
		FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();
		if (FMAGameplayEffectContext* MAContext = static_cast<FMAGameplayEffectContext*>(ContextHandle.Get()))
		{
			bIsCriticalHit = MAContext->IsCriticalHit();
		}
		AActor* TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		
		AMAPlayerController* AttackerPC = nullptr;
		if (AActor* Instigator = Data.EffectSpec.GetContext().GetOriginalInstigator())
		{
			if (APawn* Pawn = Cast<APawn>(Instigator))	AttackerPC = Cast<AMAPlayerController>(Pawn->GetController());
			else AttackerPC = Cast<AMAPlayerController>(Instigator);
		}

		AMAPlayerController* VictimPC = nullptr;
		if (TargetActor)
		{
			if (APawn* Pawn = Cast<APawn>(TargetActor))	VictimPC = Cast<AMAPlayerController>(Pawn->GetController());
			else VictimPC = Cast<AMAPlayerController>(TargetActor);
		}

		if (AttackerPC && AttackerPC!= VictimPC)
		{
			AttackerPC->ClientShowDamageNumber(FinalDamage,TargetActor,bIsCriticalHit,false);
		}
		if (VictimPC)
		{
			VictimPC->ClientShowDamageNumber(FinalDamage,TargetActor,bIsCriticalHit,true);
		}

		if (FMAGameplayEffectContext* MAContext = static_cast<FMAGameplayEffectContext*>(ContextHandle.Get()))
		{
			UMASkillAbility* SkillAbility = MAContext->GetSkillEventAbility();
			UMASkillModuleInstance* SkillEventScope = MAContext->GetSkillEventScope();
			if (SkillAbility && SkillEventScope)
			{
				FGameplayEventData EventData;
				EventData.Instigator = ContextHandle.GetOriginalInstigator();
				EventData.Target = TargetActor;
				EventData.EventMagnitude = FinalDamage;
				if (const FHitResult* HitResult = ContextHandle.GetHitResult())
				{
					EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(*HitResult);
				}
				SkillEventScope->BroadcastScopedEvent(FGameplayTag::RequestGameplayTag(TEXT("Event.Skill.DamageDealt")), EventData);
			}
		}
	}
}

DEFINE_REPNOTIFY(Health)
DEFINE_REPNOTIFY(MaxHealth)
DEFINE_REPNOTIFY(Attack)
DEFINE_REPNOTIFY(DamageVariance)
DEFINE_REPNOTIFY(MoveSpeed)
DEFINE_REPNOTIFY(AttackSpeed)
DEFINE_REPNOTIFY(Armor)
DEFINE_REPNOTIFY(ArmorPenetration)
DEFINE_REPNOTIFY(Fury)
DEFINE_REPNOTIFY(MaxFury)
DEFINE_REPNOTIFY(CriticalChance)
DEFINE_REPNOTIFY(CriticalDamage)
