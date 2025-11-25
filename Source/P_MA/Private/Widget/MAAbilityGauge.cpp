// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAAbilityGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"

#include "Widget/SkillDragDropOperation.h"
#include "Player/MAPlayerCharacter.h"
#include "Inventory/SkillBookComponent.h"
#include "Inventory/MAItemTypes.h" // [필수]

void UMAAbilityGauge::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (CooldownCounterText)
	{
		CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	}

	OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());

	WholeNumberFormattionOptions.MaximumFractionalDigits = 0;
	TwoDigitNumberFormattingOptions.MaximumFractionalDigits = 1;

	if (Icon)
	{
		Icon->GetDynamicMaterial(); 
	}
}

void UMAAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	UMAAbilitySlotDataObject* DataItem = Cast<UMAAbilitySlotDataObject>(ListItemObject);

	if (DataItem)
	{
		this->AssignedInputID = DataItem->InputID;
		UpdateSlot(DataItem->AbilityClass);
	}
}

// [통합] 드롭 이벤트
bool UMAAbilityGauge::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	USkillDragDropOperation* SkillOp = Cast<USkillDragDropOperation>(InOperation);
    
	if (SkillOp && SkillOp->SkillClass)
	{
		if (APawn* OwnerPawn = GetOwningPlayerPawn())
		{
			if (AMAPlayerCharacter* MAChar = Cast<AMAPlayerCharacter>(OwnerPawn))
			{
				if (USkillBookComponent* SkillBook = MAChar->GetSkillBookComponent())
				{
					SkillBook->EquipSkill(SkillOp->SkillClass, AssignedInputID);
					UpdateSlot(SkillOp->SkillClass);
					return true;
				}
			}
		}
	}
    
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UMAAbilityGauge::UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass)
{
	// [변경] 새로운 구조체 FSkillItemData 사용
	const FSkillItemData* WidgetData = FindWidgetDataForAbility(NewSkillClass);

	if (WidgetData && Icon)
	{
		// 부모 구조체(FBaseItemData)에 Icon이 있으므로 접근 가능
		UTexture2D* Texture = WidgetData->Icon.LoadSynchronous();
		if (Texture)
		{
			Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParamName, Texture);
			Icon->SetVisibility(ESlateVisibility::Visible);
			Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.f);
		}
	}
	else
	{
		// 스킬이 없거나 데이터가 없으면...
		if (Icon) 
		{
			// 빈 슬롯 처리 (투명하게 하거나 기본 아이콘 설정)
			// Icon->SetVisibility(ESlateVisibility::Hidden); // 필요 시 주석 해제
		}
	}

	InitializeAbility(NewSkillClass);
}

// [핵심 변경] 데이터 테이블 검색 로직
const FSkillItemData* UMAAbilityGauge::FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
	if (!AbilityDataTable) return nullptr;

	for (auto& RowPair : AbilityDataTable->GetRowMap())
	{
		// FSkillItemData로 캐스팅
		const FSkillItemData* Data = reinterpret_cast<const FSkillItemData*>(RowPair.Value);
		
		// GrantedAbility가 찾는 스킬과 같은지 확인
		if (Data && Data->GrantedAbility == AbilityClass)
		{
			return Data;
		}
	}
	return nullptr;
}

// ... (InitializeAbility, Cooldown 관련 함수들은 기존 로직 그대로 유지) ...
void UMAAbilityGauge::InitializeAbility(TSubclassOf<UGameplayAbility> NewAbilityClass)
{
	if (!NewAbilityClass)
	{
		AbilityCDO = nullptr;
		return;
	}

	AbilityCDO = NewAbilityClass->GetDefaultObject<UGameplayAbility>();

	UMAGameplayAbility_SkillBase* SkillCDO = Cast<UMAGameplayAbility_SkillBase>(AbilityCDO);
	if (SkillCDO && OwnerASC.IsValid())
	{
		SharedCooldownTag = SkillCDO->GetSharedCooldownTag();
		if (SharedCooldownTag.IsValid())
		{
			OwnerASC->RegisterGameplayTagEvent(SharedCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UMAAbilityGauge::OnCooldownTagChanged);
		}
	}

	float CooldownDuration = UMAAbilitySystemStatics::GetStaticCooldownDurationForAbility(AbilityCDO);
	float Cost = UMAAbilitySystemStatics::GetStaticCostForAbility(AbilityCDO);

	if(CooldownDurationText) CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	if(CostText) CostText->SetText(FText::AsNumber(Cost));
}

void UMAAbilityGauge::OnCooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		if (!OwnerASC.IsValid()) return;
		
		FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAllOwningTags(FGameplayTagContainer(SharedCooldownTag));

		TArray<TTuple<float,float>> Durations = OwnerASC->GetActiveEffectsTimeRemainingAndDuration(Query);
		if (Durations.Num() > 0)
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

void UMAAbilityGauge::StartCooldown(float CooldownTimeRemaining, float CooldownDuration)
{
	if(CooldownDurationText) CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	
	CachedCooldownDuration = CooldownDuration;
	CachedCooldownTimeRemaining = CooldownTimeRemaining;

	if(CooldownCounterText) CooldownCounterText->SetVisibility(ESlateVisibility::Visible);

	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UMAAbilityGauge::CooldownFinished, CachedCooldownTimeRemaining);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerUpdateHandle, this, &UMAAbilityGauge::UpdateCooldown, CooldownUpdateInterval, true, 0.f);
}

void UMAAbilityGauge::CooldownFinished()
{
	CachedCooldownDuration = CachedCooldownTimeRemaining = 0.f;
	if(CooldownCounterText) CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
	GetWorld()->GetTimerManager().ClearTimer(CooldownTimerUpdateHandle);
	
	if(Icon)
	{
		Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.f);
	}
}

void UMAAbilityGauge::UpdateCooldown()
{
	CachedCooldownTimeRemaining -= CooldownUpdateInterval;
	
	if (CachedCooldownTimeRemaining <= 0.f)
	{
		CooldownFinished();
		return;
	}

	FNumberFormattingOptions* FormattingOptions = CachedCooldownTimeRemaining > 1 ? &WholeNumberFormattionOptions : &TwoDigitNumberFormattingOptions;
	if(CooldownCounterText) CooldownCounterText->SetText(FText::AsNumber(CachedCooldownTimeRemaining, FormattingOptions));

	if (Icon && CachedCooldownDuration > 0.f)
	{
		Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.0f - CachedCooldownTimeRemaining / CachedCooldownDuration);
	}
}