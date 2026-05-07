#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASkillIconWidget.generated.h"

class UImage;

UCLASS()
class P_MA_API UMASkillIconWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SkillIconImage;
};
