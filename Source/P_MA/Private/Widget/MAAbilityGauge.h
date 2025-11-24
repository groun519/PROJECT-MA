// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h" 
#include "Engine/DataTable.h"
#include "Widget/MAAbilityListView.h"
#include "Blueprint/IUserObjectListEntry.h" 
#include "MAAbilityGauge.generated.h"

class UGameplayAbility;
class UImage;
class UTextBlock;

/**
 * 
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
 * 
 * 
 */
UCLASS()
class UMAAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	
	void UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass);
	
	const struct FAbilityWidgetData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	EMAAbilityInputID AssignedInputID;
	
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	class UDataTable* AbilityDataTable;

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