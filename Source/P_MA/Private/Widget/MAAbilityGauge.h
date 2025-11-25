// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h" 
#include "Engine/DataTable.h"
#include "Widget/MAAbilityListView.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Inventory/MAItemTypes.h" // [필수] 새로운 구조체 사용
#include "MAAbilityGauge.generated.h"

class UGameplayAbility;
class UImage;
class UTextBlock;

// [삭제] FAbilityWidgetData 구조체는 이제 MAItemTypes.h의 FSkillItemData로 대체됩니다.
/*
USTRUCT(BlueprintType)
struct FAbilityWidgetData : public FTableRowBase
{
   ...
};
*/

UCLASS()
class UMAAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    
	void UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass);
    
	// [변경] 반환 타입 수정: FAbilityWidgetData -> FSkillItemData
	const struct FSkillItemData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	EMAAbilityInputID AssignedInputID;
    
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* AbilityDataTable; // 여기에 DT_Skills를 넣게 됩니다.

private:
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
    
	UPROPERTY()
	class UGameplayAbility* AbilityCDO;

	FGameplayTag SharedCooldownTag;
	TWeakObjectPtr<class UAbilitySystemComponent> OwnerASC;

	UFUNCTION()
	void OnCooldownTagChanged(const FGameplayTag CooldownTag, int32 NewCount);
    
	void StartCooldown(float CooldownTimeRemaining, float CooldownDuration);
	void CooldownFinished();
	void UpdateCooldown();
    
	void InitializeAbility(TSubclassOf<UGameplayAbility> NewAbilityClass);

	float CachedCooldownDuration;
	float CachedCooldownTimeRemaining;

	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;

	FNumberFormattingOptions WholeNumberFormattionOptions;
	FNumberFormattingOptions TwoDigitNumberFormattingOptions;
};