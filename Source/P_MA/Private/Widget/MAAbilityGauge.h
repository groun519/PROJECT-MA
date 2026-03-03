// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GAS/MAGameplayAbilityTypes.h" 
#include "Engine/DataTable.h"
#include "Widget/MAAbilityListView.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Inventory/MAItemTypes.h"
#include "MAAbilityGauge.generated.h"

class UGameplayAbility;
class UImage;
class UTextBlock;
class UInputAction; // 💡 입력 액션을 쓰기 위해 전방 선언 추가

UCLASS()
class UMAAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
    void UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass);
    const struct FSkillData* FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const;

    // 💡 핫키 관련 함수
    void UpdateHotKeyText();
    
    UFUNCTION(BlueprintPure, Category = "UI")
    FString GetShortKeyName(FKey Key) const;

protected:
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    void HandleComboStartEvent(const struct FGameplayEventData* Payload);
    void HandleComboEndEvent(const struct FGameplayEventData* Payload);
    void HandleComboIconReadyEvent(const struct FGameplayEventData* Payload);

    void UpdateComboWait();
    void ComboWaitFinished();
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
    EMAAbilityInputID AssignedInputID;
    
    // 💡 추가: 슬롯(Skill1 등)과 입력 액션(IA_Skill1 등)을 연결해줄 맵
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TMap<EMAAbilityInputID, class UInputAction*> InputActionMapping;
    
    UPROPERTY(EditDefaultsOnly, Category = "Data")
    class UDataTable* AbilityDataTable;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
    float CooldownUpdateInterval = 0.02f;

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    FName IconMaterialParamName = "Icon";

    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    FName CooldownPercentParamname = "Percent";
    UPROPERTY(EditDefaultsOnly, Category = "Visual")
    FName ComboPercentParamName = "Percent";

    UPROPERTY(meta=(BindWidget))
    class UImage* Icon;
    UPROPERTY(meta=(BindWidget))
    class UImage* ComboGauge;

    // 💡 핫키 텍스트 위젯
    UPROPERTY(meta=(BindWidget))
    class UTextBlock* HotKeyName;

    UPROPERTY(meta=(BindWidget))
    class UTextBlock* CooldownCounterText;
    UPROPERTY(meta=(BindWidget))
    class UTextBlock* ComboCounterText;

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

    float CurrentDisplayMaxCooldown = -1.f;
    void UpdateMaxCooldownText();

    FDelegateHandle ComboStartHandle;
    FDelegateHandle ComboEndHandle;
    FDelegateHandle ComboIconReadyHandle;
    
    FTimerHandle ComboWaitTimerHandle;
    float CachedComboWaitDuration = 0.f;
    float CachedComboWaitTimeRemaining = 0.f;
    bool bIsComboWaiting = false;
};