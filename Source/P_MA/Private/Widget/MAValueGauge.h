#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "MAValueGauge.generated.h"

UCLASS()
class UMAValueGauge : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	void Bind3Attributes(
		UAbilitySystemComponent* ASC,
		const FGameplayAttribute& HealthAttribute,
		const FGameplayAttribute& MaxHealthAttribute,
		const FGameplayAttribute& ShieldAttribute);
	void Set3Values(float NewHealth, float NewMaxHealth, float NewShield);

private:
	void SetFillRatio(UWidget* FillRoot, float FillRatio);
	void HealthChanged(const FOnAttributeChangeData& ChangedData);
	void MaxHealthChanged(const FOnAttributeChangeData& ChangedData);
	void ShieldChanged(const FOnAttributeChangeData& ChangedData);

	/** Color **/
	UPROPERTY(EditAnywhere, Category = "Visual")
	FLinearColor HealthColor = FLinearColor::Red;
	UPROPERTY(EditAnywhere, Category = "Visual")
	FLinearColor ShieldColor = FLinearColor(0.1f, 0.5f, 1.f, 1.f);

	/** Text **/
	UPROPERTY(EditAnywhere, Category = "Visual")
	bool bShowValueText = true;
	UPROPERTY(EditAnywhere, Category = "Visual", meta=(ClampMin="1.0", UIMin="1.0"))
	float ValueTextSize = 20.f;

	/** Health **/
	UPROPERTY(meta = (BindWidget))
	UWidget* HealthFillRoot;
	UPROPERTY(meta = (BindWidget))
	class UImage* HealthFillImage;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	/** Shield **/
	UPROPERTY(meta = (BindWidget))
	UWidget* ShieldFillRoot;
	UPROPERTY(meta = (BindWidget))
	UImage* ShieldFillImage;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	/** EmptyField(LostHealth) **/
	UPROPERTY(meta = (BindWidget))
	class USpacer* EmptyFillSpacer;
	
	/** Caches **/
	float CachedHealth = 0.f;
	float CachedMaxHealth = 0.f;
	float CachedShield = 0.f;
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
};
