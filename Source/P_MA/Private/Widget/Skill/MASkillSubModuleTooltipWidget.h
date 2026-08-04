#pragma once

#include "CoreMinimal.h"
#include "Widget/MADescriptionTooltipWidget.h"
#include "MASkillSubModuleTooltipWidget.generated.h"

class UBorder;
class UImage;
class UMASkillModule;

UCLASS()
class P_MA_API UMASkillSubModuleTooltipWidget : public UMADescriptionTooltipWidget
{
	GENERATED_BODY()

public:
	void SetSubModule(const UMASkillModule& SubModule);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> BackgroundBorder;
};
