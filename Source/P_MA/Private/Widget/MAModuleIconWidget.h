#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "MAModuleIconWidget.generated.h"

struct FMAIconData;

/** Displays resolved module icon data through the shared module icon material. */
UCLASS()
class P_MA_API UMAModuleIconWidget : public UImage
{
	GENERATED_BODY()

public:
	void SetIconData(const FMAIconData& IconData);
};
