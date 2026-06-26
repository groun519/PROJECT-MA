#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MADestinationInfoWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class P_MA_API UMADestinationInfoWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetEnvIcon(UTexture2D* Icon);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> EnvIconImage;
};
