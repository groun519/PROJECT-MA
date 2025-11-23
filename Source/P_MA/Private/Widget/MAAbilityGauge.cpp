// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/MAAbilityGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Ability/MAGameplayAbility_SkillBase.h"

// [추가] 드래그 앤 드롭 및 플레이어 관련 헤더
#include "Widget/SkillDragDropOperation.h"
#include "Player/MAPlayerCharacter.h"
#include "Inventory/SkillBookComponent.h"

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

	// 처음 시작할 때 비어있는 상태로 시작하거나 기본값 처리
	// (아이콘 Material 인스턴스 생성 등)
	if (Icon)
	{
		Icon->GetDynamicMaterial(); 
	}
}

// 리스트 뷰 등에서 사용될 때 호출됨 (기존 유지)
void UMAAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	// [변경] UGameplayAbility가 아니라, UMAAbilitySlotDataObject를 받습니다.
	UMAAbilitySlotDataObject* DataItem = Cast<UMAAbilitySlotDataObject>(ListItemObject);

	if (DataItem)
	{
		// 1. 리스트 뷰가 지정해준 InputID(예: Q키)를 내 ID로 설정
		this->AssignedInputID = DataItem->InputID;

		// 2. 해당 슬롯의 스킬 정보로 UI 업데이트 (없으면 nullptr 들어감 -> 빈 슬롯 처리)
		UpdateSlot(DataItem->AbilityClass);
	}
}

// [통합] 드롭 이벤트 처리 (SlotWidget의 기능 이식)
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
					// 1. 스킬북에 장착 요청
					SkillBook->EquipSkill(SkillOp->SkillClass, AssignedInputID);
                    
					// 2. 내 UI(아이콘 및 쿨타임 로직) 즉시 업데이트
					UpdateSlot(SkillOp->SkillClass);
                    
					return true;
				}
			}
		}
	}
    
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

// [통합] 외부에서 슬롯을 업데이트할 때 (드롭 or 로드 시)
void UMAAbilityGauge::UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass)
{
	// [중요] 스킬이 없어도 숨기지 않도록 로직 수정
	
	// 1. 데이터 테이블에서 이미지 찾기
	const FAbilityWidgetData* WidgetData = FindWidgetDataForAbility(NewSkillClass);

	if (WidgetData && Icon)
	{
		// 스킬이 있으면 해당 아이콘 설정
		UTexture2D* Texture = WidgetData->Icon.LoadSynchronous();
		if (Texture)
		{
			Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParamName, Texture);
			Icon->SetVisibility(ESlateVisibility::Visible); // 보이게 설정
			
			// 쿨타임 셰이더 초기화 (스킬이 바뀌었으니 밝게)
			Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.f);
		}
	}
	else
	{
		// [변경] 스킬이 없거나 데이터를 못 찾았을 때
		if (Icon) 
		{
			// 방법 A: 빈 슬롯 이미지를 보여주고 싶다면 여기서 기본 텍스처 설정
			// UTexture2D* DefaultSlotTexture = ...; 
			// Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParamName, DefaultSlotTexture);
			
			// 방법 B: 그냥 비어있는 상태로 유지하되 '위젯 자체'는 숨기지 않음
			// (만약 아이콘 이미지가 검은색 배경이라면 그대로 둠)
			
			// 핵심: Hidden으로 끄지 않음! 
			// 대신 명확하게 "비어있음"을 표현하고 싶다면 투명도를 조절하거나 
			// Material 파라미터로 빈 텍스처를 넣으세요.
			
			// 예시: 아이콘을 반투명하게 하거나 기본값으로 리셋
			// Icon->SetOpacity(0.5f); 
			// Icon->SetVisibility(ESlateVisibility::Visible); // 여전히 Visible 유지
		}
	}

	// 2. 쿨타임 및 내부 로직 초기화 (스킬이 nullptr이면 내부에서 처리됨)
	InitializeAbility(NewSkillClass);
}

// [내부 함수] 쿨타임 시스템 연결 및 값 갱신
void UMAAbilityGauge::InitializeAbility(TSubclassOf<UGameplayAbility> NewAbilityClass)
{
	if (!NewAbilityClass)
	{
		AbilityCDO = nullptr;
		return;
	}

	AbilityCDO = NewAbilityClass->GetDefaultObject<UGameplayAbility>();

	// 이전 쿨타임 델리게이트 해제는 복잡하므로, 
	// 새 태그를 등록하기 전에 기존 태그 핸들 관리가 필요하지만
	// 간단하게는 새로운 태그 이벤트를 추가 등록하는 방식(중복 방지 로직 필요)을 쓰거나
	// 위젯이 갱신될 때마다 전체를 다시 그리는 방식을 씁니다.
	// 여기서는 간결함을 위해 바로 등록 로직으로 진행합니다.

	UMAGameplayAbility_SkillBase* SkillCDO = Cast<UMAGameplayAbility_SkillBase>(AbilityCDO);
	if (SkillCDO && OwnerASC.IsValid())
	{
		SharedCooldownTag = SkillCDO->GetSharedCooldownTag();
		if (SharedCooldownTag.IsValid())
		{
			// 이미 등록된 핸들러가 있다면 제거하는 로직이 있으면 좋음.
			// 일단은 새로 등록
			OwnerASC->RegisterGameplayTagEvent(SharedCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UMAAbilityGauge::OnCooldownTagChanged);
		}
	}

	// 정적 정보(쿨타임, 비용) 표시
	float CooldownDuration = UMAAbilitySystemStatics::GetStaticCooldownDurationForAbility(AbilityCDO);
	float Cost = UMAAbilitySystemStatics::GetStaticCostForAbility(AbilityCDO);

	if(CooldownDurationText) CooldownDurationText->SetText(FText::AsNumber(CooldownDuration));
	if(CostText) CostText->SetText(FText::AsNumber(Cost));
}


void UMAAbilityGauge::OnCooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		if (!OwnerASC.IsValid())
			return;
		
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
		// 쿨타임 끝났으니 밝게 표시 (Param = 1.0f)
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
		// 0(쿨타임 꽉참) ~ 1(쿨타임 없음)
		// Shader 로직에 따라 방향이 다를 수 있음. 보통 1->0 진행 or 0->1 진행.
		// 기존 코드: 1.0f - (Remaining / Duration) -> 시간이 지날수록 1에 가까워짐 (게이지가 차오름)
		Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.0f - CachedCooldownTimeRemaining / CachedCooldownDuration);
	}
}

// [통합] 헬퍼 함수 구현
const FAbilityWidgetData* UMAAbilityGauge::FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
	if (!AbilityDataTable) return nullptr;

	for (auto& RowPair : AbilityDataTable->GetRowMap())
	{
		const FAbilityWidgetData* Data = reinterpret_cast<const FAbilityWidgetData*>(RowPair.Value);
		if (Data && Data->AbilityClass == AbilityClass)
		{
			return Data;
		}
	}
	return nullptr;
}