#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MASkillModuleDragVisualWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class P_MA_API UMASkillModuleDragVisualWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetIcon(UTexture2D* InIconTexture, FLinearColor InIconColor);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> DragIconImage;
};
