#include "Setting/MAGameSettings.h"

#include "GAS/Elemental/MAElementalConfigData.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/PA_AbilitySystemGenerics.h"
#include "GAS/Skill/Module/MAModuleQualityData.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"

UMAGameSettings::UMAGameSettings()
{
	PlayerHitDamageTextStyle.Color = FLinearColor::Red;
	CriticalDamageTextStyle.Color = FLinearColor(1.f, 0.82f, 0.15f, 1.f);
	ReverseCriticalDamageTextStyle.Color = FLinearColor(0.25f, 0.25f, 0.25f, 1.f);
	HealDamageTextStyle.Color = FLinearColor::Green;
	HealDamageTextStyle.ZOffset = 130.f;
	ShieldAttributeFeedbackTextStyle.Color = FLinearColor(0.5f, 0.75f, 1.f, 1.f);
	CoinAttributeFeedbackTextStyle.Color = FLinearColor(1.f, 0.72f, 0.08f, 1.f);
	CoinAttributeFeedbackTextStyle.ZOffset = 120.f;
	FireDamageTextStyle.Color = FLinearColor(1.f, 0.35f, 0.05f, 1.f);
	IceDamageTextStyle.Color = FLinearColor(0.f, 1.f, 1.f);
}

const UPA_AbilitySystemGenerics* UMAGameSettings::GetAbilitySystemGenerics() const
{
	return AbilitySystemGenerics.LoadSynchronous();
}

const UDataTable* UMAGameSettings::GetPlayerBaseStatDataTable() const
{
	return PlayerBaseStatDataTable.LoadSynchronous();
}

const UDataTable* UMAGameSettings::GetMonsterBaseStatDataTable() const
{
	return MonsterBaseStatDataTable.LoadSynchronous();
}

const UDataTable* UMAGameSettings::GetElementalDataTable() const
{
	return ElementalDataTable.LoadSynchronous();
}

const UDataTable* UMAGameSettings::GetAreaDecalDataTable() const
{
	return AreaDecalDataTable.LoadSynchronous();
}

const UDataTable* UMAGameSettings::GetWarningTextDataTable() const
{
	return WarningTextDataTable.LoadSynchronous();
}

const UMAModuleQualityData* UMAGameSettings::GetModuleQualityData() const
{
	return ModuleQualityData.LoadSynchronous();
}

const UMAElementalConfigData* UMAGameSettings::GetElementalConfigData() const
{
	return ElementalConfigData.LoadSynchronous();
}

UMaterialInterface* UMAGameSettings::GetOverlayMaterial() const
{
	return OverlayMaterial.LoadSynchronous();
}

FMADamageTextStyle UMAGameSettings::GetDamageTextStyle(const FGameplayTag& DamageTypeTag, EMADamageCriticalResult CriticalResult, bool bIsPlayerHit) const
{
	FMADamageTextStyle Style = DefaultDamageTextStyle;
	if (DamageTypeTag.MatchesTag(UMAAbilitySystemStatics::GetHealDamageTypeTag()))
	{
		Style = HealDamageTextStyle;
	}
	else if (DamageTypeTag.MatchesTag(UMAAbilitySystemStatics::GetFireDamageTypeTag()))
	{
		Style = FireDamageTextStyle;
	}
	else if (DamageTypeTag.MatchesTag(UMAAbilitySystemStatics::GetIceDamageTypeTag()))
	{
		Style = IceDamageTextStyle;
	}
	else if (DamageTypeTag == UMAAbilitySystemStatics::GetDefaultDamageTypeTag() && CriticalResult == EMADamageCriticalResult::Critical)
	{
		Style = CriticalDamageTextStyle;
	}
	else if (DamageTypeTag == UMAAbilitySystemStatics::GetDefaultDamageTypeTag() && CriticalResult == EMADamageCriticalResult::ReverseCritical)
	{
		Style = ReverseCriticalDamageTextStyle;
	}
	else if (bIsPlayerHit)
	{
		Style = PlayerHitDamageTextStyle;
	}

	if (DamageTypeTag == UMAAbilitySystemStatics::GetFixedDamageTypeTag())
	{
		Style.OutlineColor = FixedDamageOutlineColor;
	}
	return Style;
}
