#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAHUD.generated.h"

class UHealthBarWidget;
class UMAAttackSlotWidget;
class UHorizontalBox;

UCLASS()
class P_MA_API UMAHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;

    void UpdateHealth(float CurrentHealth, float MaxHealth);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UHealthBarWidget> HealthBarWidgetClass;

    UPROPERTY(meta = (BindWidget))
    UHealthBarWidget* HealthBarWidget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<UMAAttackSlotWidget> AttackSlotWidgetClass;

    UPROPERTY(meta = (BindWidget))
    UHorizontalBox* HorizontalBox_SkillSlots;

private:
    void CreateSkillSlots(int32 NumSlots);
};