#include "GAS/MAGameplayAbilityTypes.h"

bool FMAGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	Super::NetSerialize(Ar, Map, bOutSuccess);

	uint32 RepBits =0;
	if (Ar.IsSaving())
	{
		if (bIsCriticalHit) RepBits |=1 << 0;
	}
	Ar.SerializeBits(&RepBits, 1);

	if (Ar.IsLoading())
	{
		bIsCriticalHit = (RepBits & (1<<0)) != 0;
	}
	bOutSuccess = true;
	return true;	
}

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
	BaseGold{0.f},
	BaseCriticalChance{0.f},
	BaseCriticalDamage{0.f}
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
