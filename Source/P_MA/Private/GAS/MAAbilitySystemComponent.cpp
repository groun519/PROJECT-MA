#include "GAS/MAAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayEffectExtension.h"
#include "GAS/MAGameplayAbilityTypes.h"
#include "GAS/MAAttributeSet.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "GAS/PA_AbilitySystemGenerics.h"
#include "Player/MAPlayerController.h"
#include "Player/MAPlayerCharacter.h"

static AMAPlayerController* ResolvePlayerControllerFromActor(AActor* Actor)
{
	if (APawn* Pawn = Cast<APawn>(Actor))
	{
		return Cast<AMAPlayerController>(Pawn->GetController());
	}

	return Cast<AMAPlayerController>(Actor);
}

UMAAbilitySystemComponent::UMAAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UMAAttributeSet::GetHealthAttribute()).AddUObject(this, &UMAAbilitySystemComponent::HealthUpdated);
}

void UMAAbilitySystemComponent::ApplyInitialEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	if (!AbilitySystemGenerics)
		return;

	for (const TSubclassOf<UGameplayEffect>& EffectClass : AbilitySystemGenerics->GetInitialEffects())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
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

	if (!AbilitySystemGenerics) return;

	for (const TSubclassOf<UGameplayAbility>& PassiveAbility : AbilitySystemGenerics->GetPassiveAbilities())
	{
		if (!PassiveAbility)
		{
			UE_LOG(LogTemp, Warning, TEXT("Skipped invalid passive ability. Owner=%s"), *GetNameSafe(GetOwner()));
			continue;
		}

		GiveAbility(FGameplayAbilitySpec(PassiveAbility, 1, -1, nullptr));
	}
}

void UMAAbilitySystemComponent::ApplyFullStatEffect()
{
	if (!AbilitySystemGenerics) return;
	AuthApplyGameplayEffect(AbilitySystemGenerics->GetFullStatEffect());
}

void UMAAbilitySystemComponent::ApplyReviveStatEffect()
{
	if (!AbilitySystemGenerics) return;
	AuthApplyGameplayEffect(AbilitySystemGenerics->GetReviveStatEffect());
}

void UMAAbilitySystemComponent::NotifyDamageAppliedFromGameplayEffect(const FGameplayEffectModCallbackData& Data)
{
	const FGameplayEffectContextHandle ContextHandle = Data.EffectSpec.GetContext();
	const FMAGameplayEffectContext* MAContext = static_cast<const FMAGameplayEffectContext*>(ContextHandle.Get());
	if (!MAContext || MAContext->GetDisplayMagnitude() <= 0.f) return;

	FMADamageAppliedEvent DamageAppliedEvent;
	DamageAppliedEvent.SourceActor = ContextHandle.GetOriginalInstigator();
	DamageAppliedEvent.TargetActor = Data.Target.AbilityActorInfo ? Data.Target.AbilityActorInfo->AvatarActor.Get() : nullptr;
	DamageAppliedEvent.DisplayMagnitude = MAContext->GetDisplayMagnitude();
	DamageAppliedEvent.DamageTypeTag = MAContext->GetDamageTypeTag().IsValid()
		? MAContext->GetDamageTypeTag()
		: UMAAbilitySystemStatics::GetDefaultDamageTypeTag();
	DamageAppliedEvent.CriticalResult = MAContext->GetCriticalResult();
	if (const FHitResult* HitResult = ContextHandle.GetHitResult())
	{
		DamageAppliedEvent.HitResult = *HitResult;
	}

	UMAAbilitySystemComponent* SourceASC = Cast<UMAAbilitySystemComponent>(ContextHandle.GetOriginalInstigatorAbilitySystemComponent());
	UMAAbilitySystemComponent* TargetASC = Cast<UMAAbilitySystemComponent>(&Data.Target);
	if (SourceASC && SourceASC != TargetASC)
	{
		SourceASC->ShowDamageText(DamageAppliedEvent, false);
	}
	if (TargetASC)
	{
		TargetASC->ShowDamageText(DamageAppliedEvent, true);
	}
}

void UMAAbilitySystemComponent::ShowDamageText(const FMADamageAppliedEvent& DamageAppliedEvent, bool bIsIncoming) const
{
	AActor* TargetActor = DamageAppliedEvent.TargetActor.Get();
	if (!TargetActor) return;

	AActor* ViewerActor = bIsIncoming ? TargetActor : DamageAppliedEvent.SourceActor.Get();
	if (!ViewerActor || (!bIsIncoming && ViewerActor == TargetActor)) return;

	if (AMAPlayerController* PlayerController = ResolvePlayerControllerFromActor(ViewerActor))
	{
		PlayerController->ClientShowDamageNumber(
			DamageAppliedEvent.DisplayMagnitude,
			TargetActor,
			DamageAppliedEvent.CriticalResult,
			bIsIncoming,
			DamageAppliedEvent.DamageTypeTag);
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

/** 
 * @brief 체력 변경 콜백.
 * @details 체력이 0 이하이면 서버 권한(Authority)을 확인하고 DeathEffect를 적용합니다.
 * 
 * @param ChangeData 변경된 Attribute 정보(이전/새 값).
 *
 * @note 서버에서만 상태 변화 적용.
 */
void UMAAbilitySystemComponent::HealthUpdated(const FOnAttributeChangeData& ChangeData)
{
	/** [Exception Handling] **//**
	 * 1. GetOwner()
	 *		=>	find MACharacter(AActor*)
	 *			AActorComponent's Owner automatically link (Component's attaching Actor)
	 *			
	 * 2. (!GetOwner()) return;
	 *		=>	if MAAbilitySystemComponent not have Owner, return.
	 *			(if Character is Dead)
	 */
	// if (!GetOwner()) return;
	//
	// if (ChangeData.NewValue <= 0 && GetOwner()->HasAuthority() && DeathEffect)
	// {
	// 	AuthApplyGameplayEffect(DeathEffect);
	// }
	
		// 1. 안전장치: 주인 없거나 서버 아니면 중단
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

		// --- 가득 참상태 관리 ---
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

	// --- 바닥남 상태 관리 ---
	if (ChangeData.NewValue <= 0)
	{
		if (!HasMatchingGameplayTag(UMAAbilitySystemStatics::GetHealthEmptyStatTag()))
		{
			AddLooseGameplayTag(UMAAbilitySystemStatics::GetHealthEmptyStatTag());

			// 죽음 효과는 반드시 체력이 0 이하일 때만 적용
			if(AbilitySystemGenerics && AbilitySystemGenerics->GetDeathEffect())
				AuthApplyGameplayEffect(AbilitySystemGenerics->GetDeathEffect());

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
	if (!AbilitySystemGenerics || !GetOwner())
	{
		return;
	}
	
	const UDataTable* TableToUse = nullptr;
	
	//플레이어인 경우 플레이어 데이터 테이블로 초기화
	if (Cast<AMAPlayerCharacter>(Owner))
	{
		if (!AbilitySystemGenerics->GetPlayerBaseStatDataTable())
			return;
		TableToUse = AbilitySystemGenerics->GetPlayerBaseStatDataTable();
		
		const FPlayerBaseStats* BaseStats = nullptr;
		for (const TPair<FName, uint8*>& DataPair : TableToUse->GetRowMap())
		{
			BaseStats = TableToUse->FindRow<FPlayerBaseStats>(DataPair.Key, "");
			if (BaseStats && BaseStats->Class == GetOwner()->GetClass())
			{
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
			SetNumericAttributeBase(UMAAttributeSet::GetAttackRangeAttribute(), BaseStats->BaseAttackRange);
			SetNumericAttributeBase(UMAAttributeSet::GetCoinAttribute(), BaseStats->BaseCoin);
			SetNumericAttributeBase(UMAAttributeSet::GetFocusAttribute(), BaseStats->BaseFocus);
			SetNumericAttributeBase(UMAAttributeSet::GetCriticalDamageAttribute(), BaseStats->BaseCriticalDamage);
			SetNumericAttributeBase(UMAAttributeSet::GetReverseCriticalDamageAttribute(), BaseStats->BaseReverseCriticalDamage);
		}
	}
	//몬스터인 경우 몬스터 데이터 테이블로 초기화
	else
	{
		if (!AbilitySystemGenerics->GetMonsterBaseStatDataTable())
			return;
		TableToUse = AbilitySystemGenerics->GetMonsterBaseStatDataTable();

		const FMonsterBaseStats* BaseStats = nullptr;
		for (const TPair<FName, uint8*>& DataPair : TableToUse->GetRowMap())
		{
			BaseStats = TableToUse->FindRow<FMonsterBaseStats>(DataPair.Key, "");
			if (BaseStats && BaseStats->Class == GetOwner()->GetClass())
			{
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
			AppliedBaseTags = BaseStats->BaseImmunityTags;
			AddLooseGameplayTags(BaseStats->BaseImmunityTags);
		}
	}
}

void UMAAbilitySystemComponent::ServerSideInit()
{
	InitializeBaseAttributes();
	ApplyInitialEffects();
	GiveInitialAbilities();
}
