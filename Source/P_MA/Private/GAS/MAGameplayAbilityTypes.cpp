#include "GAS/MAGameplayAbilityTypes.h"

bool FMAGameplayEffectContext::NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
{
	bool bSuccess = true;
	Super::NetSerialize(Ar, Map, bSuccess);

	uint32 RepBits =0;
	if (Ar.IsSaving())
	{
		if (CriticalResult != EMADamageCriticalResult::None) RepBits |=1 << 0;
		if (DamageTypeTag.IsValid()) RepBits |= 1 << 1;
		if (!FMath::IsNearlyZero(DisplayMagnitude)) RepBits |= 1 << 2;
	}
	Ar.SerializeBits(&RepBits, 3);

	if (Ar.IsLoading())
	{
		if ((RepBits & (1 << 0)) == 0)
		{
			CriticalResult = EMADamageCriticalResult::None;
		}
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
	if ((RepBits & (1 << 0)) != 0)
	{
		uint8 CriticalResultValue = Ar.IsSaving() ? static_cast<uint8>(CriticalResult) : 0;
		Ar.SerializeBits(&CriticalResultValue, 2);
		CriticalResult = static_cast<EMADamageCriticalResult>(CriticalResultValue);
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
	BaseFocus{0.f},
	BaseCriticalDamage{1.5f},
	BaseReverseCriticalDamage{0.5f},
	BaseAttackRange{1.f},
	BaseMoveSpeed{0.f},
	BaseArmor{0.f},
	BaseArmorPenetration{0.f},
	BaseCoin{0.f}
{
}

FMonsterBaseStats::FMonsterBaseStats()
	:Class{nullptr},
	BaseMaxHealth{0.f},
	BaseAttack{0.f},
	BaseMoveSpeed{0.f},
	BaseAttackSpeed{0.f},
	BaseAttackRange{1.f},
	BaseArmor{0.f},
	BaseArmorPenetration{0.f}
{
}
