#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MAAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class UMAAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMAAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnAttributeAggregatorCreated(const FGameplayAttribute& Attribute, FAggregator* NewAggregator) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData &Data) override;
	
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, Health)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, MaxHealth)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, Attack)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, DamageVariance)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, MoveSpeed)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, SlowMultiplier)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, AttackSpeed)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, Armor)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, ArmorPenetration)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, Fury)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, MaxFury)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, CriticalChance)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, CriticalDamage)
	ATTRIBUTE_ACCESSORS(UMAAttributeSet, Temperature)

private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)			FGameplayAttributeData Health;
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)		FGameplayAttributeData MaxHealth;
	UPROPERTY(ReplicatedUsing = OnRep_Attack)			FGameplayAttributeData Attack;
	UPROPERTY(ReplicatedUsing = OnRep_DamageVariance)	FGameplayAttributeData DamageVariance;
	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed)		FGameplayAttributeData MoveSpeed;
	UPROPERTY(ReplicatedUsing = OnRep_SlowMultiplier)	FGameplayAttributeData SlowMultiplier;
	UPROPERTY(ReplicatedUsing = OnRep_AttackSpeed)		FGameplayAttributeData AttackSpeed;
	UPROPERTY(ReplicatedUsing = OnRep_Armor)			FGameplayAttributeData Armor;
	UPROPERTY(ReplicatedUsing = OnRep_ArmorPenetration)	FGameplayAttributeData ArmorPenetration;
	UPROPERTY(ReplicatedUsing = OnRep_Fury)				FGameplayAttributeData Fury;
	UPROPERTY(ReplicatedUsing = OnRep_MaxFury)			FGameplayAttributeData MaxFury;
	UPROPERTY(ReplicatedUsing = OnRep_CriticalChance)	FGameplayAttributeData CriticalChance;
	UPROPERTY(ReplicatedUsing = OnRep_CriticalDamage)	FGameplayAttributeData CriticalDamage;
	UPROPERTY(ReplicatedUsing = OnRep_Temperature)		FGameplayAttributeData Temperature;
	
	UFUNCTION()	void OnRep_Health(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Attack(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_DamageVariance(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_SlowMultiplier(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_AttackSpeed(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Armor(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_ArmorPenetration(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Fury(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_MaxFury(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_CriticalChance(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_CriticalDamage(const FGameplayAttributeData& OldValue);
	UFUNCTION() void OnRep_Temperature(const FGameplayAttributeData& OldValue);
};
