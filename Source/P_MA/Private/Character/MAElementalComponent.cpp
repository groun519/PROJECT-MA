#include "Character/MAElementalComponent.h"

#include "Character/MACharacter.h"
#include "Character/MAOverlayComponent.h"
#include "GAS/MAAbilitySystemComponent.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Elemental/MAGameplayEffect_TemperatureRecovery.h"
#include "GAS/Elemental/MAGameplayEffect_TemperatureSlow.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/Skill/StatusEffect/MAGameplayEffect_StatusEffectDuration.h"
#include "MAMaterialParams.h"
#include "Materials/MaterialInstanceDynamic.h"

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
	if (OwnerCharacter && TemperatureOverlayMaterial)
	{
		if (UMAOverlayComponent* OverlayComponent = OwnerCharacter->GetOverlayComponent())
		{
			OverlayComponent->RemovePersistentOverlay(TemperatureOverlayMaterial);
		}
	}
	RemoveTemperatureRecovery();
	RemoveTemperatureSlow();
	RemoveFrozenStatus();
	Super::EndPlay(EndPlayReason);
}

void UMAElementalComponent::BindToASC()
{
	if (!OwnerCharacter) return;

	OwnerASC = Cast<UMAAbilitySystemComponent>(OwnerCharacter->GetAbilitySystemComponent());
	if (!OwnerASC) return;

	CurrentTemperature = OwnerASC->GetNumericAttribute(UMAAttributeSet::GetTemperatureAttribute());
	OwnerASC->GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetTemperatureAttribute()).AddUObject(this, &UMAElementalComponent::HandleTemperatureChanged);
	RefreshTemperatureOverlay();
	RefreshFrozenStatus();
	RefreshTemperatureSlow();
	RefreshTemperatureRecoveryEffect();
}

void UMAElementalComponent::HandleTemperatureChanged(const FOnAttributeChangeData& Data)
{
	CurrentTemperature = Data.NewValue;
	RefreshTemperatureOverlay();
	RefreshFrozenStatus();
	RefreshTemperatureSlow();
	RefreshTemperatureRecoveryEffect();
}

void UMAElementalComponent::RefreshTemperatureOverlay()
{
	if (!OwnerCharacter || !TemperatureOverlayMaterial) return;

	if (!TemperatureOverlayMID)
	{
		if (UMAOverlayComponent* OverlayComponent = OwnerCharacter->GetOverlayComponent())
		{
			TemperatureOverlayMID = OverlayComponent->AddPersistentOverlay(TemperatureOverlayMaterial, 2);
		}
	}
	if (!TemperatureOverlayMID) return;

	TemperatureOverlayMID->SetScalarParameterValue(PARAM_TemperatureOverlay_TemperatureAlpha, CalculateTemperatureOverlayAlpha());
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
		RemoveTemperatureRecovery();
		return;
	}

	if (!IsTemperatureRecoveryActive())
	{
		ApplyTemperatureRecovery();
	}
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

bool UMAElementalComponent::IsFrozenStatusActive() const
{
	return OwnerASC
		&& FrozenStatusEffectHandle.IsValid()
		&& OwnerASC->GetActiveGameplayEffect(FrozenStatusEffectHandle);
}

void UMAElementalComponent::RefreshFrozenStatus()
{
	if (!OwnerCharacter || !OwnerCharacter->HasAuthority() || !OwnerASC) return;

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
