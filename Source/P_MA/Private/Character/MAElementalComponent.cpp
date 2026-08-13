#include "Character/MAElementalComponent.h"

#include "Character/MACharacter.h"
#include "Character/MAOverlayComponent.h"
#include "Engine/World.h"
#include "GameplayEffectExtension.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Elemental/MAGameplayEffect_BurnDamage.h"
#include "GAS/Elemental/MAElementalConfigData.h"
#include "GAS/Elemental/MAGameplayEffect_TemperatureRecovery.h"
#include "GAS/Elemental/MAGameplayEffect_TemperatureSlow.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/Damage/MADamageApplicator.h"
#include "GAS/Skill/Damage/MASkillDamageTypes.h"
#include "GAS/Skill/Area/MASkillAreaStatics.h"
#include "GAS/Skill/Area/MASkillAreaTypes.h"
#include "GAS/Skill/StatusEffect/MAGameplayEffect_StatusEffectDuration.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Setting/MAGameSettings.h"
#include "TimerManager.h"

UMAElementalComponent::UMAElementalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMAElementalComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AMACharacter>(GetOwner());
	BindToASC();
}

void UMAElementalComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OwnerASC)
	{
		OwnerASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetColdTemperatureImmunityTag()).RemoveAll(this);
		OwnerASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetHeatTemperatureImmunityTag()).RemoveAll(this);
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TemperatureRecoveryDelayTimerHandle);
	}
	RemoveTemperatureRecovery();
	RemoveTemperatureSlow();
	RemoveBurnDamage();
	RemoveFrozenStatus();
	Super::EndPlay(EndPlayReason);
}

const UMAElementalConfigData* UMAElementalComponent::GetElementalConfigData() const
{
	return OverrideElementalConfigData ? OverrideElementalConfigData : UMAGameSettings::Get()->GetElementalConfigData();
}

void UMAElementalComponent::BindToASC()
{
	if (!OwnerCharacter) return;

	OwnerASC = Cast<UMAAbilitySystemComponent>(OwnerCharacter->GetAbilitySystemComponent());
	if (!OwnerASC) return;

	CurrentTemperature = OwnerASC->GetNumericAttribute(UMAAttributeSet::GetTemperatureAttribute());
	OwnerASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetTemperatureAttribute()).AddUObject(this, &UMAElementalComponent::HandleTemperatureChanged);
	OwnerASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetColdTemperatureImmunityTag()).AddUObject(
		this,
		&UMAElementalComponent::HandleTemperatureImmunityChanged);
	OwnerASC->RegisterGameplayTagEvent(UMAAbilitySystemStatics::GetHeatTemperatureImmunityTag()).AddUObject(
		this,
		&UMAElementalComponent::HandleTemperatureImmunityChanged);
	RefreshTemperatureOverlay();
	RefreshFrozenStatus();
	RefreshTemperatureSlow();
	RefreshBurnDamage();
	DelayTemperatureRecovery();
}

void UMAElementalComponent::HandleTemperatureImmunityChanged(FGameplayTag ImmunityTag, int32)
{
	if (ImmunityTag == UMAAbilitySystemStatics::GetColdTemperatureImmunityTag())
	{
		RefreshFrozenStatus();
		RefreshTemperatureSlow();
	}
	else if (ImmunityTag == UMAAbilitySystemStatics::GetHeatTemperatureImmunityTag())
	{
		RefreshBurnDamage();
	}
}

void UMAElementalComponent::HandleTemperatureChanged(const FOnAttributeChangeData& Data)
{
	CurrentTemperature = Data.NewValue;
	RefreshTemperatureOverlay();
	RefreshFrozenStatus();
	RefreshTemperatureSlow();
	RefreshBurnDamage(
		Data.NewValue > Data.OldValue && Data.GEModData
			? Data.GEModData->EffectSpec.GetContext()
			: FGameplayEffectContextHandle());

	const bool bFromTemperatureRecovery = Data.GEModData
		&& Data.GEModData->EffectSpec.Def
		&& Data.GEModData->EffectSpec.Def->IsA<UMAGameplayEffect_TemperatureRecovery>();
	if (bFromTemperatureRecovery)
	{
		RefreshTemperatureRecoveryEffect();
	}
	else
	{
		DelayTemperatureRecovery();
	}
}

void UMAElementalComponent::RefreshTemperatureOverlay()
{
	if (!OwnerCharacter || OwnerCharacter->GetNetMode() == NM_DedicatedServer) return;

	if (!TemperatureOverlayMID)
	{
		if (UMAOverlayComponent* OverlayComponent = OwnerCharacter->GetOverlayComponent())
		{
			TemperatureOverlayMID = OverlayComponent->GetOrCreateOverlay();
		}
	}
	if (!TemperatureOverlayMID) return;

	TemperatureOverlayMID->SetScalarParameterValue(PARAM_Overlay_TemperatureAlpha, CalculateTemperatureOverlayAlpha());
}

float UMAElementalComponent::CalculateTemperatureOverlayAlpha() const
{
	return FMath::Clamp((CurrentTemperature + 100.f) / 200.f, 0.f, 1.f);
}

void UMAElementalComponent::RefreshTemperatureRecoveryEffect()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !OwnerASC) return;

	if (FMath::IsNearlyZero(CurrentTemperature))
	{
		GetWorld()->GetTimerManager().ClearTimer(TemperatureRecoveryDelayTimerHandle);
		RemoveTemperatureRecovery();
		return;
	}

	if (!IsTemperatureRecoveryActive())
	{
		ApplyTemperatureRecovery();
	}
}

void UMAElementalComponent::DelayTemperatureRecovery()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !OwnerASC) return;

	UWorld* World = GetWorld();
	if (!World) return;

	World->GetTimerManager().ClearTimer(TemperatureRecoveryDelayTimerHandle);
	RemoveTemperatureRecovery();
	if (FMath::IsNearlyZero(CurrentTemperature)) return;

	const UMAElementalConfigData* ConfigData = GetElementalConfigData();
	World->GetTimerManager().SetTimer(
		TemperatureRecoveryDelayTimerHandle,
		this,
		&UMAElementalComponent::RefreshTemperatureRecoveryEffect,
		ConfigData ? ConfigData->TemperatureRecoveryDelay : 1.f,
		false);
}

bool UMAElementalComponent::IsTemperatureRecoveryActive() const
{
	return OwnerASC
		&& TemperatureRecoveryEffectHandle.IsValid()
		&& OwnerASC->GetActiveGameplayEffect(TemperatureRecoveryEffectHandle);
}

void UMAElementalComponent::ApplyTemperatureRecovery()
{
	if (!OwnerASC || IsTemperatureRecoveryActive()) return;

	FGameplayEffectSpecHandle SpecHandle(new FGameplayEffectSpec(
		GetDefault<UMAGameplayEffect_TemperatureRecovery>(),
		OwnerASC->MakeEffectContext(),
		1.f));
	if (!SpecHandle.IsValid()) return;

	const UMAElementalConfigData* ConfigData = GetElementalConfigData();
	SpecHandle.Data->Period = ConfigData ? FMath::Max(ConfigData->TemperatureRecoveryTickInterval, 0.01f) : 0.1f;
	SpecHandle.Data->SetSetByCallerMagnitude(
		UMAGameplayEffect_TemperatureRecovery::GetRecoveryRatioDataName(),
		ConfigData ? ConfigData->TemperatureRecoveryRatioPerTick : 0.01f);
	SpecHandle.Data->SetSetByCallerMagnitude(
		UMAGameplayEffect_TemperatureRecovery::GetRecoveryAmountDataName(),
		ConfigData ? ConfigData->TemperatureRecoveryAmountPerTick : 0.1f);
	TemperatureRecoveryEffectHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void UMAElementalComponent::RemoveTemperatureRecovery()
{
	if (OwnerASC && TemperatureRecoveryEffectHandle.IsValid())
	{
		OwnerASC->RemoveActiveGameplayEffect(TemperatureRecoveryEffectHandle);
	}
	TemperatureRecoveryEffectHandle.Invalidate();
}

void UMAElementalComponent::RefreshTemperatureSlow()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !OwnerASC) return;
	if (OwnerASC->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetColdTemperatureImmunityTag()))
	{
		RemoveTemperatureSlow();
		return;
	}

	const float SlowMultiplier = CalculateTemperatureSlowMultiplier();
	if (FMath::IsNearlyEqual(SlowMultiplier, 1.f))
	{
		RemoveTemperatureSlow();
		return;
	}

	if (!IsTemperatureSlowActive() || !FMath::IsNearlyEqual(CurrentTemperatureSlowMultiplier, SlowMultiplier))
	{
		RemoveTemperatureSlow();
		ApplyTemperatureSlow(SlowMultiplier);
	}
}

float UMAElementalComponent::CalculateTemperatureSlowMultiplier() const
{
	const UMAElementalConfigData* ConfigData = GetElementalConfigData();
	const float FrozenEnterTemperature = ConfigData ? ConfigData->FrozenEnterTemperature : -100.f;
	const float FrozenSlowMinMultiplier = ConfigData ? ConfigData->FrozenSlowMinMultiplier : 0.5f;
	const float FrozenTemperatureRange = FMath::Max(FMath::Abs(FrozenEnterTemperature), KINDA_SMALL_NUMBER);
	const float FrozenAlpha = FMath::Clamp(-CurrentTemperature / FrozenTemperatureRange, 0.f, 1.f);
	return FMath::Lerp(1.f, FrozenSlowMinMultiplier, FrozenAlpha);
}

bool UMAElementalComponent::IsTemperatureSlowActive() const
{
	return OwnerASC
		&& TemperatureSlowEffectHandle.IsValid()
		&& OwnerASC->GetActiveGameplayEffect(TemperatureSlowEffectHandle);
}

void UMAElementalComponent::ApplyTemperatureSlow(float SlowMultiplier)
{
	if (!OwnerASC || IsTemperatureSlowActive()) return;

	FGameplayEffectSpecHandle SpecHandle(new FGameplayEffectSpec(
		GetDefault<UMAGameplayEffect_TemperatureSlow>(),
		OwnerASC->MakeEffectContext(),
		1.f));
	if (!SpecHandle.IsValid()) return;

	CurrentTemperatureSlowMultiplier = SlowMultiplier;
	SpecHandle.Data->SetSetByCallerMagnitude(UMAGameplayEffect_TemperatureSlow::GetSlowMultiplierDataName(), SlowMultiplier);
	TemperatureSlowEffectHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void UMAElementalComponent::RemoveTemperatureSlow()
{
	if (OwnerASC && TemperatureSlowEffectHandle.IsValid())
	{
		OwnerASC->RemoveActiveGameplayEffect(TemperatureSlowEffectHandle);
	}
	TemperatureSlowEffectHandle.Invalidate();
	CurrentTemperatureSlowMultiplier = 1.f;
}

void UMAElementalComponent::RefreshBurnDamage(const FGameplayEffectContextHandle& SourceContext)
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !OwnerASC) return;
	if (OwnerASC->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetHeatTemperatureImmunityTag()))
	{
		bOverheated = false;
		RemoveBurnDamage();
		return;
	}

	if (CurrentTemperature <= 0.f)
	{
		bOverheated = false;
		RemoveBurnDamage();
		return;
	}

	const UMAElementalConfigData* ConfigData = GetElementalConfigData();
	const float OverheatEnterTemperature = ConfigData ? ConfigData->OverheatEnterTemperature : 100.f;
	const float OverheatExitTemperature = ConfigData ? ConfigData->OverheatExitTemperature : 80.f;
	const bool bShouldOverheat = bOverheated
		? CurrentTemperature > OverheatExitTemperature
		: CurrentTemperature >= OverheatEnterTemperature;
	const bool bOverheatChanged = bOverheated != bShouldOverheat;
	const bool bEnteredOverheat = !bOverheated && bShouldOverheat;

	FGameplayEffectContextHandle BurnSourceContext = SourceContext;
	if (!BurnSourceContext.IsValid())
	{
		if (const FActiveGameplayEffect* BurnEffect = OwnerASC->GetActiveGameplayEffect(BurnDamageEffectHandle))
		{
			BurnSourceContext = BurnEffect->Spec.GetContext();
		}
		else
		{
			BurnSourceContext = OwnerASC->MakeEffectContext();
		}
	}

	bOverheated = bShouldOverheat;
	if (bOverheatChanged && IsBurnDamageActive())
	{
		RemoveBurnDamage();
	}

	const float MaxDamagePerTick = ConfigData
		? (bOverheated ? ConfigData->OverheatedBurnDamagePerTick : ConfigData->MaxBurnDamagePerTick)
		: (bOverheated ? 10.f : 5.f);
	const float TickInterval = ConfigData
		? (bOverheated ? ConfigData->OverheatedBurnTickInterval : ConfigData->BurnTickInterval)
		: (bOverheated ? 0.5f : 1.f);
	// Reapplying the stack updates its source context without resetting the current burn tick.
	if (!IsBurnDamageActive() || SourceContext.IsValid())
	{
		BurnDamageEffectHandle = ApplyBurnDamageEffect(BurnSourceContext, MaxDamagePerTick, TickInterval);
	}

	if (bEnteredOverheat)
	{
		TriggerOverheatExplosion(BurnSourceContext);
	}
}

bool UMAElementalComponent::IsBurnDamageActive() const
{
	return OwnerASC && BurnDamageEffectHandle.IsValid() && OwnerASC->GetActiveGameplayEffect(BurnDamageEffectHandle);
}

FActiveGameplayEffectHandle UMAElementalComponent::ApplyBurnDamageEffect(
	const FGameplayEffectContextHandle& SourceContext,
	float MaxDamagePerTick,
	float TickInterval) const
{
	if (!OwnerASC || MaxDamagePerTick <= 0.f) return FActiveGameplayEffectHandle();

	FGameplayEffectSpecHandle SpecHandle(new FGameplayEffectSpec(
		GetDefault<UMAGameplayEffect_BurnDamage>(),
		SourceContext.Duplicate(),
		1.f));
	if (!SpecHandle.IsValid()) return FActiveGameplayEffectHandle();

	SpecHandle.Data->Period = FMath::Max(TickInterval, 0.01f);
	SpecHandle.Data->SetSetByCallerMagnitude(
		UMAGameplayEffect_BurnDamage::GetMaxBurnDamageDataName(),
		MaxDamagePerTick);
	if (const UMAElementalConfigData* ConfigData = GetElementalConfigData())
	{
		SpecHandle.Data->AppendDynamicAssetTags(ConfigData->BurnGameplayCueTags);
	}
	return OwnerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void UMAElementalComponent::RemoveBurnDamage()
{
	if (OwnerASC && BurnDamageEffectHandle.IsValid())
	{
		OwnerASC->RemoveActiveGameplayEffect(BurnDamageEffectHandle);
	}
	BurnDamageEffectHandle.Invalidate();
}

void UMAElementalComponent::TriggerOverheatExplosion(const FGameplayEffectContextHandle& SourceContext)
{
	const UMAElementalConfigData* ConfigData = GetElementalConfigData();
	FGameplayEffectContextHandle ExplosionContext = SourceContext;
	if (!ExplosionContext.IsValid() && OwnerASC)
	{
		ExplosionContext = OwnerASC->MakeEffectContext();
	}
	UAbilitySystemComponent* SourceASC = ExplosionContext.GetOriginalInstigatorAbilitySystemComponent();
	if (!OwnerCharacter || !SourceASC || !ConfigData || ConfigData->OverheatExplosionRadius <= 0.f) return;

	FMASkillAreaShape AreaConfig;
	AreaConfig.Shape = EMASkillAreaShape::Circle;
	AreaConfig.Circle.Radius = ConfigData->OverheatExplosionRadius;

	const FMASkillWorldAreaShape Area = AreaConfig.ResolveWorld(
		OwnerCharacter->GetActorTransform(),
		MASkillAreaStatics::ResolveAreaScale(
			SourceASC,
			UMAAttributeSet::GetAreaRangeScaleAttribute()));
	MADamageApplicator::ApplyArea(
		ExplosionContext,
		*OwnerCharacter,
		Area,
		ConfigData->OverheatExplosionDamages);
}

bool UMAElementalComponent::IsFrozenStatusActive() const
{
	return OwnerASC
		&& FrozenStatusEffectHandle.IsValid()
		&& OwnerASC->GetActiveGameplayEffect(FrozenStatusEffectHandle);
}

void UMAElementalComponent::RefreshFrozenStatus()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !OwnerASC) return;
	if (OwnerASC->HasMatchingGameplayTag(UMAAbilitySystemStatics::GetColdTemperatureImmunityTag()))
	{
		RemoveFrozenStatus();
		return;
	}

	const UMAElementalConfigData* ConfigData = GetElementalConfigData();
	const float FrozenEnterTemperature = ConfigData ? ConfigData->FrozenEnterTemperature : -100.f;
	const float FrozenExitTemperature = ConfigData ? ConfigData->FrozenExitTemperature : -80.f;
	if (IsFrozenStatusActive())
	{
		if (CurrentTemperature >= FrozenExitTemperature)
		{
			RemoveFrozenStatus();
		}
		return;
	}

	if (CurrentTemperature <= FrozenEnterTemperature)
	{
		ApplyFrozenStatus();
	}
}

void UMAElementalComponent::ApplyFrozenStatus()
{
	if (!OwnerASC || IsFrozenStatusActive()) return;

	FGameplayEffectSpecHandle SpecHandle(new FGameplayEffectSpec(
		GetDefault<UMAGameplayEffect_StatusEffectInfinite>(),
		OwnerASC->MakeEffectContext(),
		1.f));
	if (!SpecHandle.IsValid()) return;

	SpecHandle.Data->DynamicGrantedTags.AddTag(UMAAbilitySystemStatics::GetFrozenStatTag());
	SpecHandle.Data->DynamicGrantedTags.AddTag(UMAAbilitySystemStatics::GetMoveBlockTag());
	SpecHandle.Data->DynamicGrantedTags.AddTag(UMAAbilitySystemStatics::GetRotationLockTag());
	SpecHandle.Data->DynamicGrantedTags.AddTag(UMAAbilitySystemStatics::GetAbilityBlockTag());
	FrozenStatusEffectHandle = OwnerASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void UMAElementalComponent::RemoveFrozenStatus()
{
	if (OwnerASC && FrozenStatusEffectHandle.IsValid())
	{
		OwnerASC->RemoveActiveGameplayEffect(FrozenStatusEffectHandle);
	}
	FrozenStatusEffectHandle.Invalidate();
}
