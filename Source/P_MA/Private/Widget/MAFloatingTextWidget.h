#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MAFloatingTextWidget.generated.h"

class UTextBlock;

UCLASS()
class UMAFloatingTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDisplayText(const FText& Text, const FLinearColor& Color, const FLinearColor& OutlineColor = FLinearColor::Transparent);

protected:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* DamageText;

	UPROPERTY(Transient, meta=(BindWidgetAnim))
	UWidgetAnimation* FadeUpAnim;
};
