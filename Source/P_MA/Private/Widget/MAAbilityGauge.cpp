// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAAbilityGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"

void UMAAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct();
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	//UAbilitySystemComponent*
	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
	/*
	if (OwnerASC)
	{
		OwnerASC->AbilityCommittedCallbacks.AddUObject(this, &UMAAbilityGauge::AbilityCommitted);
	}
	*/
	WholeNumberFormattionOptions.MaximumFractionalDigits = 0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits = 1;
}


void UMAAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	AbilityCDO = Cast<UGameplayAbility>(ListItemObject);

	UMAGameplayAbility_SkillBase* SkillCDO = Cast<UMAGameplayAbility_SkillBase>(AbilityCDO);
	if (SkillCDO && OwnerASC.IsValid())
	{
		SharedCooldownTag = SkillCDO->SharedCooldownTag;
		if (SharedCooldownTag.IsValid())
		{
			OwnerASC->RegisterGameplayTagEvent(SharedCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UMAAbilityGauge::OnCooldownTagChanged);
		}
	}

	float CooldownDuration = UMAAbilitySystemStatics::GetStaticCooldownDurationForAbility(AbilityCDO);
	float Cost = UMAAbilitySystemStatics::GetStaticCostForAbility(AbilityCDO);

	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	CostText->SetText(FText::AsNumber(Cost));
}

void UMAAbilityGauge::ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData)
{
	if (Icon && WidgetData)
	{
		Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParamName, WidgetData->Icon.LoadSynchronous());
	}
}

void UMAAbilityGauge::OnCooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount)
{
	if (NewCount>0)
	{
		if (!OwnerASC.IsValid())
			return;
		
		FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAllOwningTags(FGameplayTagContainer(SharedCooldownTag));

		TArray<TTuple<float,float>> Durations = OwnerASC->GetActiveEffectsTimeRemainingAndDuration(Query);
		if (Durations.Num()>0)
		{
			const float CooldownTimeRemaining = Durations[0].Get<0>();
			const float CooldownDuration = Durations[0].Get<1>();
			StartCooldown(CooldownTimeRemaining, CooldownDuration);
		}
	}
	else
	{
		CooldownFinished();
	}
}

/*
void UMAAbilityGauge::AbilityCommitted(UGameplayAbility* Ability)
{
	if (Ability->GetClass()->GetDefaultObject() == AbilityCDO)
	{
		float CooldownTimeRemaining = 0.f;
		float CooldownDuration = 0.f;

		Ability->GetCooldownTimeRemainingAndDuration(Ability->GetCurrentAbilitySpecHandle(), Ability->GetCurrentActorInfo(), CooldownTimeRemaining, CooldownDuration);

		StartCooldown(CooldownTimeRemaining, CooldownDuration);
	}
}
*/
void UMAAbilityGauge::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	CachedCooldownDuration = CooldownDuration;
	CachedCooldownTimeRemaining = CooldownTimeRemaining;

	CooldownCounterText->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UMAAbilityGauge::CooldownFinished, CachedCooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerUpdateHandle, this, &UMAAbilityGauge::UpdateCooldown, CooldownUpdateInterval, true, 0.f);
}

void UMAAbilityGauge::CooldownFinished()
{
	CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;
	CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.f);
}

void UMAAbilityGauge::UpdateCooldown()
{
	CachedCooldownTimeRemaining -= CooldownUpdateInterval;
	FNumberFormattingOptions* FormattingOptions = CachedCooldownTimeRemaining > 1 ? &WholeNumberFormattionOptions : &TwoDigitNumberFormattingOptions;
	CooldownCounterText->SetText(FText::AsNumber(CachedCooldownTimeRemaining, FormattingOptions));

	Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.0f - CachedCooldownTimeRemaining / CachedCooldownDuration);
}
