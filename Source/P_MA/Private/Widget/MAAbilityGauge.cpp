// Fill out your copyright notice in the Description page of Project Settings.

#include "Widget/MAAbilityGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GAS/MAAbilitySystemStatics.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "GAS/Ability/MAGameplayAbility_Skill.h"
#include "GAS/Setting/MASkillSubsystem.h"
#include "Widget/SkillDragDropOperation.h"
#include "Player/MAPlayerCharacter.h"
#include "Inventory/SkillBookComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "InputMappingContext.h"      
#include "Inventory/MAItemTypes.h" 
#include "InputAction.h" 

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

   if (OwnerASC.IsValid())
   {
      ComboStartHandle = OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(FGameplayTag::RequestGameplayTag("UI.Skill.ComboWaitStart")).AddUObject(this, &UMAAbilityGauge::HandleComboStartEvent);
      ComboEndHandle = OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(FGameplayTag::RequestGameplayTag("UI.Skill.ComboWaitEnd")).AddUObject(this, &UMAAbilityGauge::HandleComboEndEvent);
      ComboIconReadyHandle = OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(FGameplayTag::RequestGameplayTag("UI.Skill.ComboIconReady")).AddUObject(this, &UMAAbilityGauge::HandleComboIconReadyEvent);
   }
}

void UMAAbilityGauge::NativeDestruct()
{
   if (OwnerASC.IsValid())
   {
      OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(FGameplayTag::RequestGameplayTag("UI.Skill.ComboWaitStart")).Remove(ComboStartHandle);
      OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(FGameplayTag::RequestGameplayTag("UI.Skill.ComboWaitEnd")).Remove(ComboEndHandle);
      OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(FGameplayTag::RequestGameplayTag("UI.Skill.ComboIconReady")).Remove(ComboIconReadyHandle);
   }
   Super::NativeDestruct();
}

void UMAAbilityGauge::NativeOnListItemObjectSet(UObject* ListItemObject)
{
   if (!OwnerASC.IsValid())
   {
      OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwningPlayerPawn());
        
      if (OwnerASC.IsValid())
      {
         if (!ComboStartHandle.IsValid())
         {
            ComboStartHandle = OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(FGameplayTag::RequestGameplayTag("UI.Skill.ComboWaitStart")).AddUObject(this, &UMAAbilityGauge::HandleComboStartEvent);
            ComboEndHandle = OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(FGameplayTag::RequestGameplayTag("UI.Skill.ComboWaitEnd")).AddUObject(this, &UMAAbilityGauge::HandleComboEndEvent);
            ComboIconReadyHandle = OwnerASC->GenericGameplayEventCallbacks.FindOrAdd(FGameplayTag::RequestGameplayTag("UI.Skill.ComboIconReady")).AddUObject(this, &UMAAbilityGauge::HandleComboIconReadyEvent);
         }
      }
   }
   
    UMAAbilitySlotDataObject* DataItem = Cast<UMAAbilitySlotDataObject>(ListItemObject);
    if (DataItem)
    {
       this->AssignedInputID = DataItem->InputID;
       
       UpdateHotKeyText();
       
       UpdateSlot(DataItem->AbilityClass);
    }
}

void UMAAbilityGauge::UpdateHotKeyText()
{
    if (!HotKeyName) return;

    FString DisplayText = TEXT(""); 

    APlayerController* PC = GetOwningPlayer();
    if (PC && PC->GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (UInputAction** FoundAction = InputActionMapping.Find(AssignedInputID))
            {
                if (UInputAction* Action = *FoundAction)
                {
                    // 현재 IMC에서 이 액션에 할당된 키 목록 가져오기
                    TArray<FKey> MappedKeys = Subsystem->QueryKeysMappedToAction(Action);
                    
                    if (MappedKeys.Num() > 0)
                    {
                        // 할당된 키가 있다면 짧은 이름으로 변환해서 텍스트로 지정
                        DisplayText = GetShortKeyName(MappedKeys[0]);
                    }
                }
            }
        }
    }

    HotKeyName->SetText(FText::FromString(DisplayText));
}

FString UMAAbilityGauge::GetShortKeyName(FKey Key) const
{
    if (Key == EKeys::RightMouseButton) return TEXT("MouseR");
    if (Key == EKeys::LeftMouseButton) return TEXT("MouseL");
    if (Key == EKeys::SpaceBar) return TEXT("Space");
    return Key.GetDisplayName().ToString();
}

void UMAAbilityGauge::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    UpdateMaxCooldownText();
}

bool UMAAbilityGauge::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    // 1. 기존 로직: 스킬북(UI)에서 스킬을 드래그해서 슬롯에 놓는 경우
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

    // =========================================================================
    // 2. [신규 로직]: 바닥에 있는 아이템(MAFieldItem)을 바로 스킬 슬롯에 놓는 경우
    // =========================================================================
    UInventoryItemDragDropOp* InventoryOp = Cast<UInventoryItemDragDropOp>(InOperation);
    if (InventoryOp)
    {
        if (AMAFieldItem* FieldItem = Cast<AMAFieldItem>(InventoryOp->Payload))
        {
            if (FieldItem->ItemDataTable && !FieldItem->ItemRowName.IsNone())
            {
                // 데이터 테이블에서 이 아이템이 '스킬 데이터'를 가지고 있는지 확인합니다.
                const FSkillItemData* SkillData = FieldItem->ItemDataTable->FindRow<FSkillItemData>(FieldItem->ItemRowName, TEXT("CheckSkillDrop"));
                
                // 타입이 스킬이 맞다면 진행!
                if (SkillData && SkillData->ItemType == EMAItemType::Skill)
                {
                    if (AMAPlayerCharacter* PlayerChar = Cast<AMAPlayerCharacter>(GetOwningPlayerPawn()))
                    {
                        // 1) 캐릭터에게 스킬 획득 처리 (기존에 쓰시던 TryPurchaseSkill)
                        if (UInventoryComponent* InvComp = PlayerChar->GetComponentByClass<UInventoryComponent>())
                        {
                            InvComp->TryPurchaseSkill(FieldItem->ItemRowName, FieldItem->ItemDataTable);
                        }
                        
                        // 2) 지금 마우스를 놓은 '이 슬롯'에 스킬을 바로 장착시킴
                        if (USkillBookComponent* SkillBook = PlayerChar->GetSkillBookComponent())
                        {
                            if (SkillData->GrantedAbility) // 데이터 테이블에 등록된 어빌리티 클래스
                            {
                                SkillBook->EquipSkill(SkillData->GrantedAbility, AssignedInputID);
                                UpdateSlot(SkillData->GrantedAbility); // UI 아이콘 업데이트
                            }
                        }

                        // 3) 처리가 끝났으니 바닥의 액터는 파괴
                        FieldItem->Destroy();
                        return true;
                    }
                }
            }
        }
    }
    
    return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UMAAbilityGauge::HandleComboStartEvent(const struct FGameplayEventData* Payload)
{
   if (!Payload || !Payload->OptionalObject || !AbilityCDO) return;

   const UMAGameplayAbility_Skill* TriggeredSkill = Cast<UMAGameplayAbility_Skill>(Payload->OptionalObject);
   if (TriggeredSkill && TriggeredSkill->GetClass() == AbilityCDO->GetClass())
   {
      if (GetWorld()->GetTimerManager().IsTimerActive(ComboWaitTimerHandle))
         return;

      CachedComboWaitDuration = Payload->EventMagnitude;
      CachedComboWaitTimeRemaining = CachedComboWaitDuration;
      bIsComboWaiting = true;
      
      ComboCounterText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

      GetWorld()->GetTimerManager().SetTimer(ComboWaitTimerHandle, this, &UMAAbilityGauge::UpdateComboWait, CooldownUpdateInterval, true, 0.f);
   }
}

void UMAAbilityGauge::HandleComboEndEvent(const struct FGameplayEventData* Payload)
{
   if (!Payload || !Payload->OptionalObject || !AbilityCDO) return;

   const UMAGameplayAbility_Skill* TriggeredSkill = Cast<UMAGameplayAbility_Skill>(Payload->OptionalObject);
   if (TriggeredSkill && TriggeredSkill->GetClass() == AbilityCDO->GetClass())
   {
      ComboWaitFinished();
   }
}

void UMAAbilityGauge::HandleComboIconReadyEvent(const struct FGameplayEventData* Payload)
{
   if (!Payload || !Payload->OptionalObject || !AbilityCDO) return;

   const UMAGameplayAbility_Skill* TriggeredSkill = Cast<UMAGameplayAbility_Skill>(Payload->OptionalObject);
   if (TriggeredSkill && TriggeredSkill->GetClass() == AbilityCDO->GetClass())
   {
      const FBehavior_Combo* ComboConfig = TriggeredSkill->GetComboData().ModuleConfig.GetPtr<FBehavior_Combo>();
      if (ComboConfig && ComboConfig->ComboIcon && Icon)
      {
         Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParamName, ComboConfig->ComboIcon);
         Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.f);
            
         bIsComboWaiting = true; 
         
         CostText->SetVisibility(ESlateVisibility::Hidden);
         CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
         CooldownCounterText->SetVisibility(ESlateVisibility::Hidden); 
         
         ComboGauge->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
         ComboGauge->GetDynamicMaterial()->SetScalarParameterValue(ComboPercentParamName, 1.f);
      }
   }
}

void UMAAbilityGauge::UpdateComboWait()
{
   if (!bIsComboWaiting) return;

   CachedComboWaitTimeRemaining -= CooldownUpdateInterval;

   if (CachedComboWaitTimeRemaining <= 0.f)
   {
      ComboWaitFinished();
      return;
   }

   if (ComboGauge && CachedComboWaitDuration > 0.f)
   {
      ComboGauge->GetDynamicMaterial()->SetScalarParameterValue(ComboPercentParamName, CachedComboWaitTimeRemaining / CachedComboWaitDuration);
   }

   FNumberFormattingOptions* FormattingOptions = CachedComboWaitTimeRemaining > 1.f ? &WholeNumberFormattionOptions : &TwoDigitNumberFormattingOptions;
   ComboCounterText->SetText(FText::AsNumber(CachedComboWaitTimeRemaining, FormattingOptions));
}

void UMAAbilityGauge::ComboWaitFinished()
{
   if (!bIsComboWaiting) return;
   bIsComboWaiting = false;

   GetWorld()->GetTimerManager().ClearTimer(ComboWaitTimerHandle);

   ComboGauge->SetVisibility(ESlateVisibility::Hidden);
   ComboCounterText->SetVisibility(ESlateVisibility::Hidden);

   const FSkillItemData* WidgetData = FindWidgetDataForAbility(AbilityCDO->GetClass());
   if (WidgetData && Icon)
   {
      UTexture2D* Texture = WidgetData->Icon.LoadSynchronous();
      if (Texture)
      {
         Icon->GetDynamicMaterial()->SetTextureParameterValue(IconMaterialParamName, Texture);
      }
   }

   CostText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
   if (CooldownDurationText && CurrentDisplayMaxCooldown > 0.f) 
   {
      CooldownDurationText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
   }

   if (CachedCooldownTimeRemaining > 0.f && CachedCooldownDuration > 0.f)
   {
      CooldownCounterText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
      Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.0f - CachedCooldownTimeRemaining / CachedCooldownDuration);
   }
   else 
   {
      CooldownCounterText->SetVisibility(ESlateVisibility::Hidden);
      Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.f);
   }
}

void UMAAbilityGauge::UpdateSlot(TSubclassOf<UGameplayAbility> NewSkillClass)
{
    const FSkillItemData* WidgetData = FindWidgetDataForAbility(NewSkillClass);

    if (WidgetData && Icon)
    {
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
       // Icon->SetVisibility(ESlateVisibility::Hidden);
    }

    InitializeAbility(NewSkillClass);
}

const FSkillItemData* UMAAbilityGauge::FindWidgetDataForAbility(const TSubclassOf<UGameplayAbility>& AbilityClass) const
{
    if (!AbilityDataTable) return nullptr;

    for (auto& RowPair : AbilityDataTable->GetRowMap())
    {
       const FSkillItemData* Data = reinterpret_cast<const FSkillItemData*>(RowPair.Value);
       if (Data && Data->GrantedAbility == AbilityClass)
       {
          return Data;
       }
    }
    return nullptr;
}

void UMAAbilityGauge::InitializeAbility(TSubclassOf<UGameplayAbility> NewAbilityClass)
{
    if (!NewAbilityClass)
    {
       AbilityCDO = nullptr;
       SharedCooldownTag = FGameplayTag();
       return;
    }

    AbilityCDO = NewAbilityClass->GetDefaultObject<UGameplayAbility>();
    if (UMAGameplayAbility_Skill* SkillCDO = Cast<UMAGameplayAbility_Skill>(AbilityCDO))
    {
       SharedCooldownTag = FGameplayTag::EmptyTag;

       if (UWorld* World = GetWorld())
       {
          if (UMASkillSubsystem* SkillSys = World->GetGameInstance()->GetSubsystem<UMASkillSubsystem>())
          {
             const FSkillData* SkillData = SkillSys->GetSkillData(SkillCDO->GetSkillID());
             if (SkillData && SkillData->CooldownTag.IsValid())
             {
                SharedCooldownTag = SkillData->CooldownTag;
             }
          }
       }
       if (SharedCooldownTag.IsValid() && OwnerASC.IsValid())
       {
          OwnerASC->RegisterGameplayTagEvent(SharedCooldownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UMAAbilityGauge::OnCooldownTagChanged);
       }
    }

    UpdateMaxCooldownText();
    float Cost = UMAAbilitySystemStatics::GetStaticCostForAbility(AbilityCDO);
    if (CostText)  CostText->SetText(FText::AsNumber(Cost));
}

void UMAAbilityGauge::UpdateMaxCooldownText()
{
    if (!AbilityCDO || !OwnerASC.IsValid())
       return;

    float NewMaxCooldown = UMAAbilitySystemStatics::GetExpectedCooldownDuration(AbilityCDO,OwnerASC.Get());
    if (!FMath::IsNearlyEqual(CurrentDisplayMaxCooldown,NewMaxCooldown))
    {
       CurrentDisplayMaxCooldown = NewMaxCooldown;
       if (CooldownDurationText)
       {
          if (CurrentDisplayMaxCooldown>0.f)
          {
             CooldownDurationText->SetVisibility(ESlateVisibility::Visible);
             CooldownDurationText->SetText(FText::AsNumber(CurrentDisplayMaxCooldown));
          }
          else
          {
             CooldownDurationText->SetVisibility(ESlateVisibility::Hidden);
          }
       }
    }
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

    if (!bIsComboWaiting && Icon && CachedCooldownDuration > 0.f)
    {
       Icon->GetDynamicMaterial()->SetScalarParameterValue(CooldownPercentParamname, 1.0f - CachedCooldownTimeRemaining / CachedCooldownDuration);
    }
}