#include "GAS/MAAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayEffectExtension.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/Skill/Damage/MASkillDamageApplicator.h"
#include "GAS/PA_AbilitySystemGenerics.h"
#include "Player/MAPlayerController.h"
#include "Player/MAPlayerCharacter.h"
#include "Player/Feedback/MAFloatingTextComponent.h"
#include "Setting/MAGameSettings.h"

UMAAbilitySystemComponent::UMAAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetHealthAttribute()).AddUObject(this, &UMAAbilitySystemComponent::HealthUpdated);
}

void UMAAbilitySystemComponent::GiveInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	for (const TSubclassOf<UGameplayAbility>& Ability : Abilities)
	{
		if (!Ability)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipped invalid initial ability. Owner=%s"), *GetNameSafe(GetOwner()));
			continue;
		}

		GiveAbility(FGameplayAbilitySpec(Ability, 0, INDEX_NONE, nullptr));
	}

	for (const TSubclassOf<UGameplayAbility>& Ability : BasicAbilities)
	{
		if (!Ability)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipped invalid basic ability. Owner=%s"), *GetNameSafe(GetOwner()));
			continue;
		}

		GiveAbility(FGameplayAbilitySpec(Ability, 1, INDEX_NONE, nullptr));
	}

	const UPA_AbilitySystemGenerics* SystemGenerics = UMAGameSettings::Get()->GetAbilitySystemGenerics();
	if (SystemGenerics && SystemGenerics->GetDeadAbility())
	{
		GiveAbility(FGameplayAbilitySpec(SystemGenerics->GetDeadAbility(), 1, INDEX_NONE, nullptr));
	}
}

void UMAAbilitySystemComponent::ApplyFullStatEffect()
{
	const UPA_AbilitySystemGenerics* SystemGenerics = UMAGameSettings::Get()->GetAbilitySystemGenerics();
	if (SystemGenerics) AuthApplyGameplayEffect(SystemGenerics->GetFullStatEffect());
}

void UMAAbilitySystemComponent::ApplyReviveStatEffect()
{
	const UPA_AbilitySystemGenerics* SystemGenerics = UMAGameSettings::Get()->GetAbilitySystemGenerics();
	if (SystemGenerics) AuthApplyGameplayEffect(SystemGenerics->GetReviveStatEffect());
}

void UMAAbilitySystemComponent::NotifyDamageAppliedFromGameplayEffect(const FGameplayEffectModCallbackData& Data)
{
	const FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	if (!MAContext || !MAContext->GetDamageTypeTag().IsValid()) return;

	MASkillDamageApplicator::PostProcessAppliedDamage(*this, Data);
	const bool bIsMiss =
		MAContext->GetDamageTypeTag() == UMAAbilitySystemStatics::GetDefaultDamageTypeTag()
		&& FMath::IsNearlyZero(MAContext->GetDisplayMagnitude());
	if (MAContext->GetDisplayMagnitude() > 0.f || bIsMiss)
	{
		UMAAbilitySystemComponent* SourceASC = Cast<UMAAbilitySystemComponent>(
			ContextHandle.GetOriginalInstigatorAbilitySystemComponent());
		if (SourceASC && SourceASC != this)
		{
			SourceASC->ShowDamageText(Data, false);
		}
		ShowDamageText(Data, true);
	}
}

void UMAAbilitySystemComponent::ShowDamageText(const FGameplayEffectModCallbackData& Data, bool bIsIncoming) const
{
	const FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	if (!MAContext) return;

	AActor* TargetActor = Data.Target.AbilityActorInfo ? Data.Target.AbilityActorInfo->AvatarActor.Get() : nullptr;
	if (!TargetActor) return;

	AActor* ViewerActor = bIsIncoming ? TargetActor : ContextHandle.GetOriginalInstigator();
	if (!ViewerActor || (!bIsIncoming && ViewerActor == TargetActor)) return;

	AMAPlayerController* PlayerController = Cast<AMAPlayerController>(ViewerActor);
	if (APawn* ViewerPawn = Cast<APawn>(ViewerActor))
	{
		PlayerController = Cast<AMAPlayerController>(ViewerPawn->GetController());
	}
	if (!PlayerController) return;

	if (UMAFloatingTextComponent* FloatingText = PlayerController->GetFloatingTextComponent())
	{
		FloatingText->ClientShowDamage(
			MAContext->GetDisplayMagnitude(),
			TargetActor,
			MAContext->GetCriticalResult(),
			bIsIncoming,
			MAContext->GetDamageTypeTag());
	}
}

void UMAAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int Level)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UMAAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	// 주인 없거나 서버 아니면 중단
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	bool bFound = false;
	float MaxHealth = GetGameplayAttributeValue(UMAAttributeSet::GetMaxHealthAttribute(), bFound);

	if (bFound && ChangeData.NewValue >= MaxHealth)
	{
		// 여기서는 죽음 효과를 적용하면 안됨
		if (!HasMatchingGameplayTag(UMAAbilitySystemStatics::GetHealthFullStatTag()))
		{
			AddLooseGameplayTag(UMAAbilitySystemStatics::GetHealthFullStatTag());
		}
	}
	else
	{
		RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetHealthFullStatTag());
	}

	if (ChangeData.NewValue <= 0)
	{
		if (!HasMatchingGameplayTag(UMAAbilitySystemStatics::GetHealthEmptyStatTag()))
		{
			AddLooseGameplayTag(UMAAbilitySystemStatics::GetHealthEmptyStatTag());

			// 죽음 효과는 반드시 체력이 0 이하일 때만 적용
			const UPA_AbilitySystemGenerics* SystemGenerics = UMAGameSettings::Get()->GetAbilitySystemGenerics();
			if (SystemGenerics && SystemGenerics->GetDeathEffect())
				AuthApplyGameplayEffect(SystemGenerics->GetDeathEffect());

			FGameplayEventData DeadAbilityEventData;
			if(ChangeData.GEModData)
				DeadAbilityEventData.ContextHandle = ChangeData.GEModData->EffectSpec.GetContext();

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), UMAAbilitySystemStatics::GetDeadStatTag(), DeadAbilityEventData);
		}
	}
	else
	{
		// 체력이 0보다 클 때 태그를 제거하는 구문
		RemoveLooseGameplayTag(UMAAbilitySystemStatics::GetHealthEmptyStatTag());
	}
}

void UMAAbilitySystemComponent::InitializeBaseAttributes()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	//플레이어인 경우 플레이어 데이터 테이블로 초기화
	if (Cast<AMAPlayerCharacter>(Owner))
	{
		const UDataTable* BaseStatTable = UMAGameSettings::Get()->GetPlayerBaseStatDataTable();
		if (!BaseStatTable) return;

		const FPlayerBaseStats* BaseStats = nullptr;
		for (const TPair<FName, uint8*>& DataPair : BaseStatTable->GetRowMap())
		{
			const FPlayerBaseStats* Candidate = BaseStatTable->FindRow<FPlayerBaseStats>(DataPair.Key, "");
			if (Candidate && Candidate->Class == Owner->GetClass())
			{
				BaseStats = Candidate;
				break;
			}
		}

		if (BaseStats)
		{
			SetNumericAttributeBase(UMAAttributeSet::GetMaxHealthAttribute(), BaseStats->BaseMaxHealth);
			SetNumericAttributeBase(UMAAttributeSet::GetAttackAttribute(), BaseStats->BaseAttack);
			SetNumericAttributeBase(UMAAttributeSet::GetAttackSpeedAttribute(), BaseStats->BaseAttackSpeed);
			SetNumericAttributeBase(UMAAttributeSet::GetMoveSpeedAttribute(), BaseStats->BaseMoveSpeed);
			SetNumericAttributeBase(UMAAttributeSet::GetSlowMultiplierAttribute(), 1.f);
			SetNumericAttributeBase(UMAAttributeSet::GetArmorAttribute(), BaseStats->BaseArmor);
			SetNumericAttributeBase(UMAAttributeSet::GetArmorPenetrationAttribute(), BaseStats->BaseArmorPenetration);
			const float BaseAttackRange = BaseStats->BaseAttackRange > 0.f ? BaseStats->BaseAttackRange : 1.f;
			SetNumericAttributeBase(UMAAttributeSet::GetAttackRangeAttribute(), BaseAttackRange);
			SetNumericAttributeBase(UMAAttributeSet::GetCoinAttribute(), BaseStats->BaseCoin);
			SetNumericAttributeBase(UMAAttributeSet::GetFocusAttribute(), BaseStats->BaseFocus);
			SetNumericAttributeBase(UMAAttributeSet::GetCriticalDamageAttribute(), BaseStats->BaseCriticalDamage);
			SetNumericAttributeBase(UMAAttributeSet::GetReverseCriticalDamageAttribute(), BaseStats->BaseReverseCriticalDamage);
		}
	}
	else //몬스터인 경우 몬스터 데이터 테이블로 초기화
	{
		const UDataTable* BaseStatTable = UMAGameSettings::Get()->GetMonsterBaseStatDataTable();
		if (!BaseStatTable) return;

		const FMonsterBaseStats* BaseStats = nullptr;
		for (const TPair<FName, uint8*>& DataPair : BaseStatTable->GetRowMap())
		{
			const FMonsterBaseStats* Candidate = BaseStatTable->FindRow<FMonsterBaseStats>(DataPair.Key, "");
			if (Candidate && Candidate->Class == Owner->GetClass())
			{
				BaseStats = Candidate;
				break;
			}
		}

		if (BaseStats)
		{
			SetNumericAttributeBase(UMAAttributeSet::GetMaxHealthAttribute(), BaseStats->BaseMaxHealth);
			SetNumericAttributeBase(UMAAttributeSet::GetAttackAttribute(), BaseStats->BaseAttack);
			SetNumericAttributeBase(UMAAttributeSet::GetAttackSpeedAttribute(), BaseStats->BaseAttackSpeed);
			SetNumericAttributeBase(UMAAttributeSet::GetMoveSpeedAttribute(), BaseStats->BaseMoveSpeed);
			SetNumericAttributeBase(UMAAttributeSet::GetSlowMultiplierAttribute(), 1.f);
			const float BaseAttackRange = BaseStats->BaseAttackRange > 0.f ? BaseStats->BaseAttackRange : 1.f;
			SetNumericAttributeBase(UMAAttributeSet::GetAttackRangeAttribute(), BaseAttackRange);
			SetNumericAttributeBase(UMAAttributeSet::GetArmorAttribute(), BaseStats->BaseArmor);
			SetNumericAttributeBase(UMAAttributeSet::GetArmorPenetrationAttribute(), BaseStats->BaseArmorPenetration);
			AppliedBaseTags = BaseStats->BaseImmunityTags;
			AddLooseGameplayTags(BaseStats->BaseImmunityTags);
		}
	}
}

void UMAAbilitySystemComponent::ServerSideInit()
{
	InitializeBaseAttributes();
	ApplyFullStatEffect();
	GiveInitialAbilities();
}
