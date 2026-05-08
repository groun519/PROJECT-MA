#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASkillIconWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class P_MA_API UMASkillIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHotkeyText(const FText& InText);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SkillIconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> HotkeyText;
};
