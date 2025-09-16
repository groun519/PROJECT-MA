#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAGameplayWidget.generated.h"

class UMASkillSlotWidget;
class UMAPassiveSlotWidget;
class UHorizontalBox;
class UMAValueGauge;

UCLASS()
class UMAGameplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 체력바와 같은 기존 UI 요소 유지
	UPROPERTY(meta = (BindWidget))
	class UMAValueGauge* HealthBar;

	// 스킬 슬롯 위젯 클래스와 바인딩
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UMASkillSlotWidget> SkillSlotWidgetClass;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HorizontalBox_SkillSlots;

	// 패시브 슬롯 위젯 클래스와 바인딩
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UMAPassiveSlotWidget> PassiveSlotWidgetClass;

	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* HorizontalBox_PassiveSlots;

private:
	void CreateSkillSlots(int32 NumSlots);
	void CreatePassiveSlots(int32 NumSlots);
};
