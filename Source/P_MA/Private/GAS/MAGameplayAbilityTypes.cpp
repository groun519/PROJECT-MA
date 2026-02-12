// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MAGameplayAbilityTypes.h"


FGenericDamageEffectDef::FGenericDamageEffectDef()
	:DamageEffect{nullptr}, PushVelocity{0.f}
{
}

FPlayerBaseStats::FPlayerBaseStats()
	:Class{nullptr},
	BaseMaxHealth{0.f},
	BaseAttack{0.f},
	BaseDamageVariance{0.f},
	BaseAttackSpeed{0.f},
	BaseAttackRange{0.f},
	BaseMoveSpeed{0.f},
	BaseArmor{0.f},
	BaseArmorPenetration{0.f},
	BaseGold{0.f}
{
}

FMonsterBaseStats::FMonsterBaseStats()
	:Class{nullptr},
	BaseMaxHealth{0.f},
	BaseAttack{0.f},
	BaseDamageVariance{0.f},
	BaseMoveSpeed{0.f},
	BaseAttackSpeed{0.f},
	BaseArmor{0.f},
	BaseArmorPenetration{0.f},
	BaseFuryMax{0.f}
{
}