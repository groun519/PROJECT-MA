// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h" // InputID Enum
#include "Engine/DataTable.h"
#include "Widget/MAAbilityListView.h"
#include "Blueprint/IUserObjectListEntry.h" 
#include "MAAbilityGauge.generated.h"

class UGameplayAbility;
class UImage;
class UTextBlock;

/**
 * 데이터 테이블 구조체 (기존 유지)
 */
USTRUCT(BlueprintType)
struct FAbilityWidgetData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> AbilityClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	float Price = 0.0f;
};

/**
 * 통합된 어빌리티 게이지 위젯
 * (쿨타임 표시 + 드래그 앤 드롭 장착 기능)
 */
UCLASS()
class UMAAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	// [통합] 외부에서 스킬을 강제로 세팅하는 함수 (슬롯 갱신용)
	void UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass);

	// [통합] 데이터 테이블 헬퍼 함수
	const struct FAbilityWidgetData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

protected:
	// [통합] 드래그 앤 드롭 처리
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	// [통합] 이 슬롯이 담당하는 입력 키 (Q, E, R 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	EMAAbilityInputID AssignedInputID;

	// [통합] 아이콘 정보를 찾기 위한 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* AbilityDataTable;

private:
	// --- 기존 쿨타임 관련 변수 ---
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CooldownUpdateInterval = 0.02f;

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName IconMaterialParamName = "Icon";

	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName CooldownPercentParamname = "Percent";

	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownCounterText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownDurationText;

	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CostText;

	// 현재 슬롯에 할당된 어빌리티의 CDO (Default Object)
	UPROPERTY()
	class UGameplayAbility* AbilityCDO;

	FGameplayTag SharedCooldownTag;
	TWeakObjectPtr<class UAbilitySystemComponent> OwnerASC;

	UFUNCTION()
	void OnCooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount);
	
	void StartCooldown(float CooldownTimeRemaining, float CooldownDuration);
	void CooldownFinished();
	void UpdateCooldown();

	// 내부적으로 스킬 정보를 세팅하고 쿨타임 리스너를 등록하는 함수
	void InitializeAbility(TSubclassOf<UGameplayAbility> NewAbilityClass);

	float CachedCooldownDuration;
	float CachedCooldownTimeRemaining;

	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;

	FNumberFormattingOptions WholeNumberFormattionOptions;
	FNumberFormattingOptions TwoDigitNumberFormattingOptions;
};