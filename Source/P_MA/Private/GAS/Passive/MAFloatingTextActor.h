#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MAFloatingTextActor.generated.h"

class UWidgetComponent;

UCLASS()
class AMAFloatingTextActor : public AActor
{
	GENERATED_BODY()
	
public:
	AMAFloatingTextActor();

	void PlayText(const FText& Text, const FLinearColor& Color, const FLinearColor& OutlineColor = FLinearColor::Transparent, float Scale = 1.f);

protected:
	UPROPERTY(VisibleAnywhere)
	UWidgetComponent* FloatingTextWidgetComp;
};
