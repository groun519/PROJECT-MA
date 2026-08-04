#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayEffectTypes.h"
#include "MATemperatureGauge.generated.h"

class UAbilitySystemComponent;
class UImage;
class USpacer;
class UWidget;

UCLASS()
class P_MA_API UMATemperatureGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeDestruct() override;

	void BindTemperatureAttribute(UAbilitySystemComponent* ASC);
	void SetCriticalRatio(float NewCriticalRatio);
	void SetTemperature(float NewTemperature);

private:
	void ApplySectionRatio() const;
	void ApplyTemperatureVisuals() const;
	void HandleTemperatureChanged(const FOnAttributeChangeData& Data);
	void UnbindTemperatureAttribute();
	FLinearColor ResolveNormalTemperatureColor() const;
	FLinearColor ResolveCriticalTemperatureColor() const;
	FLinearColor ResolveProgressTemperatureColor() const;
	FLinearColor ResolveTemperatureColorBySet(
		const FLinearColor& NegativeColor,
		const FLinearColor& ZeroColor,
		const FLinearColor& PositiveColor) const;
	void SetFillRatio(UWidget* Widget, float FillRatio) const;

	UPROPERTY(EditAnywhere, Category="Visual|Normal")
	FLinearColor NormalNegativeTemperatureColor = FLinearColor(0.15f, 0.55f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, Category="Visual|Normal")
	FLinearColor NormalZeroTemperatureColor = FLinearColor(0.45f, 0.45f, 0.45f, 1.f);

	UPROPERTY(EditAnywhere, Category="Visual|Normal")
	FLinearColor NormalPositiveTemperatureColor = FLinearColor(1.f, 0.25f, 0.05f, 1.f);

	UPROPERTY(EditAnywhere, Category="Visual|Critical")
	FLinearColor CriticalNegativeTemperatureColor = FLinearColor(0.f, 0.2f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, Category="Visual|Critical")
	FLinearColor CriticalZeroTemperatureColor = FLinearColor(0.25f, 0.25f, 0.25f, 1.f);

	UPROPERTY(EditAnywhere, Category="Visual|Critical")
	FLinearColor CriticalPositiveTemperatureColor = FLinearColor(1.f, 0.f, 0.f, 1.f);

	UPROPERTY(EditAnywhere, Category="Visual|Progress")
	FLinearColor ProgressNegativeTemperatureColor = FLinearColor(0.1f, 0.45f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, Category="Visual|Progress")
	FLinearColor ProgressZeroTemperatureColor = FLinearColor(0.65f, 0.65f, 0.65f, 1.f);

	UPROPERTY(EditAnywhere, Category="Visual|Progress")
	FLinearColor ProgressPositiveTemperatureColor = FLinearColor(1.f, 0.3f, 0.05f, 1.f);

	UPROPERTY(EditAnywhere, Category="Temperature", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float CriticalRatio = 0.2f;

	UPROPERTY(EditAnywhere, Category="Temperature", meta=(ClampMin="1.0", UIMin="1.0"))
	float MaxAbsTemperature = 100.f;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> NormalImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> CriticalImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> TemperatureFillImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USpacer> EmptyFillSpacer;

	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle TemperatureChangedHandle;
	float Temperature = 0.f;
};
