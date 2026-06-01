#include "GAS/MAGameplayAbilityTypes.h"

bool FMAGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bool bSuccess = true;
	Super::NetSerialize(Ar, Map, bSuccess);

	uint32 RepBits =0;
	if (Ar.IsSaving())
	{
		if (bIsCriticalHit) RepBits |=1 << 0;
		if (DamageTypeTag.IsValid()) RepBits |= 1 << 1;
		if (!FMath::IsNearlyZero(DisplayMagnitude)) RepBits |= 1 << 2;
	}
	Ar.SerializeBits(&RepBits, 3);

	if (Ar.IsLoading())
	{
		bIsCriticalHit = (RepBits & (1<<0)) != 0;
		if ((RepBits & (1 << 1)) == 0)
		{
			DamageTypeTag = FGameplayTag();
		}
		if ((RepBits & (1 << 2)) == 0)
		{
			DisplayMagnitude = 0.f;
		}
	}

	if ((RepBits & (1 << 1)) != 0)
	{
		bool bDamageTypeTagSuccess = true;
		DamageTypeTag.NetSerialize(Ar, Map, bDamageTypeTagSuccess);
		bSuccess &= bDamageTypeTagSuccess;
	}
	if ((RepBits & (1 << 2)) != 0)
	{
		Ar << DisplayMagnitude;
	}

	bOutSuccess = bSuccess;
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
	BaseAttackSpeed{0.f},
	BaseAttackRange{0.f},
	BaseMoveSpeed{0.f},
	BaseArmor{0.f},
	BaseArmorPenetration{0.f},
	BaseCoin{0.f},
	BaseCriticalChance{0.f},
	BaseCriticalDamage{0.f}
{
}

FMonsterBaseStats::FMonsterBaseStats()
	:Class{nullptr},
	BaseMaxHealth{0.f},
	BaseAttack{0.f},
	BaseMoveSpeed{0.f},
	BaseAttackSpeed{0.f},
	BaseArmor{0.f},
	BaseArmorPenetration{0.f},
	BaseFuryMax{0.f}
{
}
